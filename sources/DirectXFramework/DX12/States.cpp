#include "pch.h"

namespace DX12
{
	bool ResourceStateManager::ResourceListStateCPU::is_used(Transitions* list)
	{
		return list->global_id == command_list_id;
	}
	
	bool  ResourceStateManager::is_used(Transitions* list)
	{
		SubResourcesCPU& s = get_state(list->id);
		return s.command_list_id == list->global_id;
	}
	
	ResourceStateManager::SubResourcesCPU& ResourceStateManager::get_cpu_state(Transitions* list)
	{
		auto& state = get_state(list->id);

		return state;
	}

	void ResourceStateManager::prepare_state(Transitions* from, const Resource* resource, SubResourcesGPU& subres)
	{
		auto cpu_state = get_state(from->id);

		for (int i = 0; i < subres.subres.size(); i++)
		{
			auto& gpu = subres.subres[i];
			auto& cpu = cpu_state.subres[i];

			if (cpu.command_list_id != from->global_id) continue;

			auto merged_state = merge_state(gpu.state, cpu.get_first_state());
			auto last_state = merged_state;
			if (cpu.first_transition != cpu.last_transition) //if no transitions - merge ti!
			{
				merged_state = cpu.get_first_state();
				last_state = cpu.get_state();
			}

			if ((GetSupportedStates(from->get_type()) & gpu.state) == gpu.state)
			if (gpu.state != merged_state)
			{
				auto prev = cpu.first_transition;
			auto point = from->create_transition(resource, i, TransitionType::ZERO);
				point->wanted_state = merged_state;
				cpu.first_transition->prev_transition = point;
			}

			subres.subres[i].state = last_state;
		}
		
	}

	
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

			auto merged_state = merge_state(gpu.state, cpu.get_first_state());
			auto last_state = merged_state;
			if(cpu.first_transition!=cpu.last_transition) //if no transitions - merge ti!
			{
				merged_state = cpu.get_first_state();
				last_state = cpu.get_state();
			}
			
			if (gpu.state != merged_state)
			{
				assert(gpu.state != ResourceState::DIFFERENT);
				assert(gpu.state != ResourceState::UNKNOWN);

				assert(cpu.get_first_state() != ResourceState::DIFFERENT);
				assert(cpu.get_first_state() != ResourceState::UNKNOWN);

				
				
				//bool was_merged = check(merged_state & subres_cpu.get_state());
				
				target.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
					static_cast<D3D12_RESOURCE_STATES>(gpu.state),
					static_cast<D3D12_RESOURCE_STATES>(merged_state),
					i));


				states = states | gpu.state | merged_state;

				for (int j = 0; j < target.size() - 1; j++)
				{
					if (target.back().Type == target[j].Type)
						if (target.back().Transition.pResource == target[j].Transition.pResource)
							if (target.back().Transition.Subresource == target[j].Transition.Subresource)
								assert(false);
				}

			}

			gpu_state.subres[i].state = last_state;

		}


		if (cpu_state.need_discard)
		{
			discards.emplace_back(const_cast<Resource*>(resource));
		}

		return states;
	}


	void ResourceStateManager::transition(Transitions* list, const Resource* resource, ResourceState state, unsigned int s, int id, uint64_t full_id) const
	{
		auto& cpu_state = get_state(id);


		auto transition_one = [&](int subres) {

			auto& subres_cpu = cpu_state.subres[subres];

	

			if (subres_cpu.command_list_id != full_id)
			{
				subres_cpu.command_list_id = full_id;
				subres_cpu.first_transition =	subres_cpu.last_transition = list->create_transition(resource, subres, TransitionType::FIRST);
				subres_cpu.last_transition->wanted_state = state;
				subres_cpu.first_transition->prev_transition = nullptr;
			}
			else
			{
				assert(state != ResourceState::DIFFERENT);
				assert(state != ResourceState::UNKNOWN);

		//		assert(subres_cpu.state != ResourceState::DIFFERENT);
		//		assert(subres_cpu.state != ResourceState::UNKNOWN);

				auto merged_state = merge_state(subres_cpu.get_state(), state);
				bool was_merged = check(merged_state & subres_cpu.get_state());

				if(was_merged)
				{
					subres_cpu.last_transition->wanted_state = merged_state;

					if (state == ResourceState::UNORDERED_ACCESS)
					{
						list->create_uav_transition(resource);
						
						auto prev = subres_cpu.last_transition;
						subres_cpu.last_transition = list->create_transition(resource, subres);
						subres_cpu.last_transition->prev_transition = prev;
						subres_cpu.last_transition->wanted_state = state;
					}
				}
				else
				{
					auto prev = subres_cpu.last_transition;
					subres_cpu.last_transition = list->create_transition(resource, subres);
					subres_cpu.last_transition->wanted_state = state;
					subres_cpu.last_transition->prev_transition = prev;
				}

	
			}	
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

	bool ResourceStateManager::transition(Transitions* from, Transitions* to, const  Resource* resource) const
	{

		
		auto& from_state = get_state(from->id);
		auto& to_state = get_state(to->id);


		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			auto& subres_to = to_state.subres[i];

			if (subres_to.is_used(to))
			if ((GetSupportedStates(from->get_type()) & subres_to.get_first_state()) != subres_to.get_first_state())
				return false;
		}
		
		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			auto& subres_to = to_state.subres[i];

			if(subres_to.is_used(to))
				transition(from,resource, subres_to.get_first_state(), i,  from->id,from->global_id);
	
		}
		
		return true;
	}

}
