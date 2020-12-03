#pragma once
namespace DX12
{
	class Resource;

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


		mutable  SpinLock states_lock;

		mutable std::map<int,SubResourcesCPU> cpu_state;

		SubResourcesCPU& get_state(int id) const
		{
			std::lock_guard<SpinLock> guard(states_lock);

			auto it = cpu_state.find(id);

			if (it == cpu_state.end())
			{
				auto & state = cpu_state[id];
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

		void aliasing(int id, uint64_t full_id) 
		{
			SubResourcesCPU& s = get_state(id);

			if (check(s.subres[0].state & ResourceState::RENDER_TARGET) || check(s.subres[0].state & ResourceState::DEPTH_WRITE))


			s.need_discard = true;
		}
		
		void process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, std::vector<Resource*> &discards, const Resource* resource, int id, uint64_t full_id);

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& target,const  Resource* resource, ResourceState state, unsigned int subres, int id, uint64_t full_id) const;

	};


}