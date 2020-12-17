#include "pch.h"

namespace DX12
{

	ResourceState ResourceStateManager::process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, std::vector<Resource*>& discards, const Resource* resource, int id, uint64_t full_id)
	{
	ResourceState states = ResourceState::COMMON;

		if (!resource) return states;

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


				states = states | gpu.state | cpu.first_state;

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

		return states;
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

	bool ResourceStateManager::transition(CommandListType type, std::vector<D3D12_RESOURCE_BARRIER>& target, const Resource* resource, int id, uint64_t full_id, int id2, uint64_t full_id2) const
	{
		auto& from_state = get_state(id);
		auto& to_state = get_state(id2);


		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			auto& subres_to = to_state.subres[i];

			if ((GetSupportedStates(type) & subres_to.first_state) != subres_to.first_state)
				return false;
		}

		
		auto transition_one = [&](int subres) {

			auto& subres_from = from_state.subres[subres];
			auto& subres_to = to_state.subres[subres];

			ResourceState prev_state = subres_from.state;


			
			if (subres_from.command_list_id != full_id)
			{
				subres_from.command_list_id = full_id;
				subres_from.first_state = subres_to.first_state;
			}
			else
			{

				assert(subres_to.first_state != ResourceState::DIFFERENT);
				assert(subres_to.first_state != ResourceState::UNKNOWN);

				assert(subres_from.state != ResourceState::DIFFERENT);
				assert(subres_from.state != ResourceState::UNKNOWN);


				if (subres_from.state != subres_to.first_state)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
						static_cast<D3D12_RESOURCE_STATES>(subres_from.state),
						static_cast<D3D12_RESOURCE_STATES>(subres_to.first_state),
						subres));
				}
			/*	else if (state == ResourceState::UNORDERED_ACCESS)
				{
					target.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get()));
				}
				*/
			}


			subres_from.state = subres_to.first_state;
		};

	
		for (int i = 0; i < gpu_state.subres.size(); i++)
			transition_one(i);

		return true;
	}

}
