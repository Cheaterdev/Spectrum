#pragma once
namespace DX12
{
	class Transitions;
	class Resource;
	enum class CommandListType : int
	{
		DIRECT = D3D12_COMMAND_LIST_TYPE_DIRECT,
		BUNDLE = D3D12_COMMAND_LIST_TYPE_BUNDLE,
		COMPUTE = D3D12_COMMAND_LIST_TYPE_COMPUTE,
		COPY = D3D12_COMMAND_LIST_TYPE_COPY,

		GENERATE_OPS
	};
	enum class ResourceState: int
	{
		COMMON = D3D12_RESOURCE_STATE_COMMON,
		VERTEX_AND_CONSTANT_BUFFER = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		INDEX_BUFFER = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		RENDER_TARGET = D3D12_RESOURCE_STATE_RENDER_TARGET,
		UNORDERED_ACCESS = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DEPTH_WRITE = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		DEPTH_READ = D3D12_RESOURCE_STATE_DEPTH_READ,
		NON_PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		STREAM_OUT = D3D12_RESOURCE_STATE_STREAM_OUT,
		INDIRECT_ARGUMENT = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		COPY_DEST = D3D12_RESOURCE_STATE_COPY_DEST,
		COPY_SOURCE = D3D12_RESOURCE_STATE_COPY_SOURCE,
		RESOLVE_DEST = D3D12_RESOURCE_STATE_RESOLVE_DEST,
		RESOLVE_SOURCE = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		GEN_READ = D3D12_RESOURCE_STATE_GENERIC_READ,
		PRESENT = D3D12_RESOURCE_STATE_PRESENT,
		PREDICATION = D3D12_RESOURCE_STATE_PREDICATION,
		RAYTRACING_STRUCTURE = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		UNKNOWN = -1,
		DIFFERENT = -2

		, GENERATE_OPS
	};


	static const ResourceState COPY_STATES = ResourceState::COPY_DEST
		| ResourceState::COPY_SOURCE;

	static const ResourceState COMPUTE_STATES = COPY_STATES
		| ResourceState::UNORDERED_ACCESS
		| ResourceState::NON_PIXEL_SHADER_RESOURCE
		| ResourceState::INDIRECT_ARGUMENT;

	static const ResourceState GRAPHIC_STATES = COMPUTE_STATES
		| ResourceState::VERTEX_AND_CONSTANT_BUFFER
		| ResourceState::INDEX_BUFFER
		| ResourceState::RENDER_TARGET
		| ResourceState::DEPTH_WRITE
		| ResourceState::DEPTH_READ
		| ResourceState::PIXEL_SHADER_RESOURCE
		| ResourceState::STREAM_OUT

		| ResourceState::RESOLVE_DEST
		| ResourceState::RESOLVE_SOURCE;

	static inline const ResourceState& GetSupportedStates(CommandListType type)
	{
		if (type == CommandListType::COPY) return COPY_STATES;
		if (type == CommandListType::COMPUTE) return COMPUTE_STATES;
		return GRAPHIC_STATES;
	}

	static inline CommandListType GetBestType(const ResourceState &states, CommandListType preferred_type)
	{
		if (states == ResourceState::COMMON) 
			return CommandListType::DIRECT;

		if ((states & COPY_STATES) == states && preferred_type == CommandListType::COPY)
		{
			return CommandListType::COPY;
		}

		if ((states & COMPUTE_STATES) == states && (preferred_type == CommandListType::COMPUTE || preferred_type == CommandListType::COPY))
		{
			return CommandListType::COMPUTE;
		}

		return CommandListType::DIRECT;
	}

	static inline bool can_merge_state(const ResourceState& source, const ResourceState& need)
	{
		if (source == need) return true;

		ResourceState merged = source | need;
		if (check(merged & (~ResourceState::GEN_READ)))
		{
			return false;
		}

		return true;
	}
	

	static inline ResourceState merge_state(const ResourceState& source, const ResourceState& need)
	{
		if (source == need) return source;
		
		ResourceState merged = source | need;
		if(check(merged & (~ResourceState::GEN_READ)))
		{
			return need;
		}

		return merged;
	}


	struct Transition
	{
	//	UINT index;
		Resource* resource;
	//	ResourceState from;
		ResourceState wanted_state;
		UINT subres;

		Transition* prev_transition = nullptr;
	};


	struct ResourceListStateCPU
	{
		Transition* first_transition = nullptr;
		Transition* last_transition = nullptr;
		
		ResourceState get_first_state()
		{
			return first_transition->wanted_state;
		}
		ResourceState get_state()
		{
			return last_transition->wanted_state;
		}

		bool used = false;
	};

	struct SubResourcesCPU
	{
		std::vector<ResourceListStateCPU> subres;
		ResourceListStateCPU all;
		bool need_discard = false;
		bool used = false;

		void reset()
		{
			used = false;

			for(auto & s:subres)
			{
				s.used = false;
			}
		}
	};


	struct ResourceListStateGPU
	{
		ResourceState state = ResourceState::UNKNOWN;
	};

	struct SubResourcesGPU
	{
		std::vector<ResourceListStateGPU> subres;

		ResourceListStateGPU all;
	};


	
	class ResourceStateManager: public ObjectState<SubResourcesCPU>
	{
		const Resource* resource;

	protected:
		mutable SubResourcesGPU gpu_state;
		virtual ~ResourceStateManager() = default;
	public:

		using ptr = std::unique_ptr<ResourceStateManager>;

		ResourceStateManager();
		SubResourcesGPU copy_gpu() const
		{
			return gpu_state;
		}
		
		void init_subres(int count, ResourceState state) const
		{
			gpu_state.subres.resize(count);
			gpu_state.all.state = state;

			for (auto& e : gpu_state.subres)
				e.state = state;

			states.set_init_func([count](SubResourcesCPU& state)
			{
					state.subres.resize(count);
			});
		}


		SubResourcesCPU& get_cpu_state(Transitions* list) const;
		
	
		bool is_used(Transitions* list) const;
		void aliasing(int id, uint64_t full_id) const
		{
			//SubResourcesCPU& s = get_state(id);
		//	if (check(s.subres[0].state & ResourceState::RENDER_TARGET) || check(s.subres[0].state & ResourceState::DEPTH_WRITE))
		//	s.need_discard = true;
		}
		
		ResourceState process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, std::vector<Resource*> &discards,  Transitions* list) const;

		void transition(Transitions* list, ResourceState state, unsigned int subres) const;
		bool transition(Transitions* from, Transitions* to) const;
		void prepare_state(Transitions* from, SubResourcesGPU& subres) const;
	};


}