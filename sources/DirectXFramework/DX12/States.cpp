#include "pch.h"

namespace DX12
{

	void ResourceStateManager::process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, std::vector<Resource*>& discards, const Resource* resource, int id, uint64_t full_id)
	{
		if (!resource) return;

		auto cpu_state = get_state(id);

		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			auto& gpu = gpu_state.subres[i];
			auto& cpu = cpu_state.subres[i];

			if (cpu.command_list_id != full_id) continue;

			if (gpu.state != cpu.first_state)
			{
				assert(gpu.state != ResourceState::DIFFERENT);
				assert(gpu.state != ResourceState::UNKNOWN);

				assert(cpu.first_state != ResourceState::DIFFERENT);
				assert(cpu.first_state != ResourceState::UNKNOWN);

				target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
					static_cast<D3D12_RESOURCE_STATES>(gpu.state),
					static_cast<D3D12_RESOURCE_STATES>(cpu.first_state),
					i));

				for (int j = 0; j < target.size() - 1; j++)
				{
					if (target.back().Type == target[j].Type)
						if (target.back().Transition.pResource == target[j].Transition.pResource)
							if (target.back().Transition.Subresource == target[j].Transition.Subresource)
								assert(false);
				}

			}

			gpu_state.subres[i].state = cpu.state;



		}


		if (cpu_state.need_discard)
		{
			discards.emplace_back(const_cast<Resource*>(resource));
		}


	}


	void ResourceStateManager::transition(std::vector<D3D12_RESOURCE_BARRIER>& target, const Resource* resource, ResourceState state, unsigned int s, int id, uint64_t full_id) const
	{
		auto& cpu_state = get_state(id);


		auto transition_one = [&](int subres) {

			auto& subres_cpu = cpu_state.subres[subres];

			ResourceState prev_state = subres_cpu.state;


			if (subres_cpu.command_list_id != full_id)
			{
				subres_cpu.command_list_id = full_id;
				subres_cpu.first_state = state;
			}
			else
			{
				assert(state != ResourceState::DIFFERENT);
				assert(state != ResourceState::UNKNOWN);

				assert(subres_cpu.state != ResourceState::DIFFERENT);
				assert(subres_cpu.state != ResourceState::UNKNOWN);


				if (subres_cpu.state != state)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
						static_cast<D3D12_RESOURCE_STATES>(subres_cpu.state),
						static_cast<D3D12_RESOURCE_STATES>(state),
						subres));

					for (int i = 0; i < target.size() - 1; i++)
					{
						if (target.back().Type == target[i].Type)
							if (target.back().Transition.pResource == target[i].Transition.pResource)
								if (target.back().Transition.Subresource == target[i].Transition.Subresource)
									assert(false);
					}

				}
				else if (state == ResourceState::UNORDERED_ACCESS)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get()));
				}

			}


			subres_cpu.state = state;
		};

		if (s == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			for (int i = 0; i < gpu_state.subres.size(); i++)
				transition_one(i);
		}
		else
		{
			transition_one(s);
		}

	}

}
