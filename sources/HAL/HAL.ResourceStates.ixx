module;


export module HAL:ResourceStates;

import <Core_defs.h>;
import Core;

import :Types;

using namespace HAL;

export
{

	namespace HAL
	{
		class Transitions;


		constexpr uint ALL_SUBRESOURCES = std::numeric_limits<uint>::max();

		enum class TransitionType :int
		{
			ZERO,
			LAST
		};

		bool IsCompatible(CommandListType a, CommandListType b);
		bool IsFullySupport(CommandListType type, const ResourceState& states);
		CommandListType Merge(CommandListType a, CommandListType b);
		std::optional<ResourceState> merge_state(const ResourceState& source, const ResourceState& need);
		std::optional<TextureLayout> merge_layout(const TextureLayout& source, const TextureLayout& need);

		enum class BarrierFlags : UINT
		{
			NONE = 0,
			BEGIN = 1,
			END = 2,
			SINGLE = BEGIN | END,
			DISCARD = 4
		};

		struct Barrier
		{
			Resource* resource;
			ResourceState before;
			ResourceState after;
			UINT subres;
			BarrierFlags flags;
		};


		class Barriers
		{
			std::vector<Barrier> barriers;

			void validate();
			CommandListType type;
		public:

			Barriers(CommandListType type);
			operator bool() const;
			void clear();
			const std::vector<Barrier>& get_barriers() const;

			void transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags = BarrierFlags::SINGLE);

		};

		struct UsagePoint;

		struct ResourceUsage
		{
			Resource* resource = nullptr;
			ResourceState wanted_state;
			UINT subres = -1;

			ResourceUsage* prev_usage = nullptr;
  			ResourceUsage* next_usage = nullptr;

			UsagePoint* point = nullptr;
			UsagePoint* last_point = nullptr;

			bool debug = false;
		};

		struct UsagePoint
		{
			uint index;
			std::list<HAL::ResourceUsage> usages;
			HAL::Barriers  transitions;
			bool start = false;
			UsagePoint* prev_point = nullptr;
			UsagePoint* next_point = nullptr;
			Transitions* cmd_list;
			BarrierSync operation;
			UsagePoint(CommandListType type);
		};

		struct ResourceListStateCPU
		{
			ResourceUsage* first_usage = nullptr;
			ResourceUsage* last_usage = nullptr;

			bool used = false;
			bool need_discard = false;

			ResourceState get_first_usage();
			ResourceState get_usage();
			void reset();
			void check_valid(const Resource* resource);
			ResourceUsage* add_usage(ResourceUsage* usage);
			ResourceUsage* set_zero_transition(ResourceUsage* usage);
		};

		struct SubResourcesGPU;
		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;

			bool used = false;

			void reset();
			CommandListType get_best_list_type_last();
			CommandListType get_best_list_type_first();
			const ResourceListStateCPU& get_subres_state(UINT id) const;
			ResourceListStateCPU& get_subres_state(UINT id);
			ResourceUsage* get_first_usage(UINT id) const;
			ResourceUsage* get_last_usage(UINT id) const;
			ResourceState get_first_state(UINT id) const;
			ResourceState get_last_state(UINT id) const;
			void merge_read_state(CommandListType type, SubResourcesGPU& state);
		};


		struct ResourceListStateGPU
		{
			TextureLayout layout;
		};

		struct SubResourcesGPU
		{
			std::vector<ResourceListStateGPU> subres;

			bool is_valid() const;
			void operator=(const TextureLayout& layout);
			CommandListType get_best_list_type();
			void set_cpu_state(const SubResourcesCPU& cpu_state);
			void set_cpu_state_first(const SubResourcesCPU& cpu_state);
			const ResourceListStateGPU& get_subres_state(UINT id) const;
			ResourceListStateGPU& get_subres_state(UINT id);
			void merge(SubResourcesCPU& other);
		};



		class ResourceStateManager : public ObjectState<SubResourcesCPU>
		{
			const Resource* resource;

		protected:
			mutable SubResourcesGPU gpu_state;


		public:
			virtual ~ResourceStateManager() = default;
			TextureLayout initial_layout;
			using ptr = std::unique_ptr<ResourceStateManager>;
			UINT get_subres_count();
			ResourceStateManager(const Resource* resource);
			SubResourcesGPU copy_gpu() const;
			void init_subres(int count, TextureLayout layout);

			SubResourcesCPU& get_cpu_state(Transitions* list) const;

			void stop_using(Transitions* list, UINT subres) const;

			bool is_used(Transitions* list) const;


#ifdef PRETRANSITIONS_FIX
			CommandListType process_transitions(Barriers& target, Transitions* list) const;
#endif

			void transition(Transitions* list, ResourceState state, unsigned int subres) const;

			void prepare_state(Transitions* from, const SubResourcesGPU& subres) const;
			void prepare_state(Transitions* from, ResourceState state) const;

			void prepare_after_state(Transitions* from, const SubResourcesGPU& subres) const;

			void alias_begin(Transitions* list) const;
			void alias_end(Transitions* list) const;

			void connect(Transitions* from, Transitions* to);

		};


	}

}
