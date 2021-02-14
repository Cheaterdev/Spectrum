#pragma once
namespace DX12
{
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
		| ResourceState::NON_PIXEL_SHADER_RESOURCE;

	static const ResourceState GRAPHIC_STATES = COMPUTE_STATES
		| ResourceState::VERTEX_AND_CONSTANT_BUFFER
		| ResourceState::INDEX_BUFFER
		| ResourceState::RENDER_TARGET
		| ResourceState::DEPTH_WRITE
		| ResourceState::DEPTH_READ
		| ResourceState::PIXEL_SHADER_RESOURCE
		| ResourceState::STREAM_OUT
		| ResourceState::INDIRECT_ARGUMENT
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

	class ResourceStateManager
	{

		struct ResourceListStateCPU
		{
			ResourceState first_state = ResourceState::UNKNOWN;
			ResourceState state = ResourceState::UNKNOWN;

			uint64_t command_list_id = -1;
		};

		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;

			ResourceListStateCPU all;

			bool need_discard = false;
			volatile uint64_t command_list_id = -1;

			volatile uint64_t used_by_command_list_id = -1;
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


	
		mutable std::array<SubResourcesCPU, 128> cpu_state;

		mutable  SpinLock states_lock;
		mutable std::map<int, SubResourcesCPU> cpu_state_map;

		SubResourcesCPU& get_state(int id) const
		{
			if(id<128)
			return cpu_state[id];
			
			
			std::lock_guard<SpinLock> guard(states_lock);

			auto it = cpu_state_map.find(id);

			if (it == cpu_state_map.end())
			{
				auto & state = cpu_state_map[id];
				state.subres.resize(gpu_state.subres.size());

				return state;
			}

			return it->second;
			
		}



	protected:
		SubResourcesGPU gpu_state;

	public:

		using ptr = std::unique_ptr<ResourceStateManager>;

		ResourceStateManager()
		{
		}

		void init_subres(int count, ResourceState state)
		{
			gpu_state.subres.resize(count);
			gpu_state.all.state = state;

			for (auto& e : gpu_state.subres)
				e.state = state;


			for (auto& e : cpu_state)
				e.subres.resize(gpu_state.subres.size());

		}

		ResourceState get_cpu_state(int id, uint64_t full_id)
		{
			auto& state = get_state(id);

			return state.subres[0].state;
		}
		
		bool is_new(int id, uint64_t full_id) const
		{
			SubResourcesCPU& s = get_state(id);

			bool result = (s.command_list_id != full_id);

			s.command_list_id = full_id;
			if(result)
			s.need_discard = false;
			return  result;
		}

		bool use_by(int id, uint64_t full_id) const
		{
			SubResourcesCPU& s = get_state(id);

			bool result = (s.used_by_command_list_id != full_id);
			s.used_by_command_list_id = full_id;
			return  result;
		}


		void aliasing(int id, uint64_t full_id) 
		{
			SubResourcesCPU& s = get_state(id);

			if (check(s.subres[0].state & ResourceState::RENDER_TARGET) || check(s.subres[0].state & ResourceState::DEPTH_WRITE))


			s.need_discard = true;
		}
		
		ResourceState process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, std::vector<Resource*> &discards, const Resource* resource, int id, uint64_t full_id);

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& target,const  Resource* resource, ResourceState state, unsigned int subres, int id, uint64_t full_id) const;
		bool transition(CommandListType type, std::vector<D3D12_RESOURCE_BARRIER>& target, const  Resource* resource, int id, uint64_t full_id, int id2, uint64_t full_id2) const;

	};


}