#include "pch.h"

namespace DX12
{


	void Barriers::validate()
	{

#ifdef DEV
		for (int j = 0; j < native.size() - 1; j++)
		{
			if (native.back().Type == native[j].Type)
				if (native.back().Transition.pResource == native[j].Transition.pResource)
					if (native.back().Transition.Subresource == native[j].Transition.Subresource)
						assert(false);
		}

#endif
	}
	Barriers::Barriers(CommandListType type):type(type)
	{
		
	}
	void Barriers::clear()
	{
		native.clear();
	}
	const std::vector<D3D12_RESOURCE_BARRIER>& Barriers::get_native() const
	{
		return native;
	}
	void Barriers::uav(Resource* resource)
	{
		native.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get()));
	}

	void Barriers::alias(Resource* from, Resource* to)
	{
		auto native_from = from ? from->get_native().Get() : nullptr;
		auto native_to = to ? to->get_native().Get() : nullptr;

		native.emplace_back(CD3DX12_RESOURCE_BARRIER::Aliasing(native_from, native_to));
	}

	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		assert(resource);

		assert(before != ResourceState::UNKNOWN);
		assert(after != ResourceState::UNKNOWN);

		assert(IsFullySupport(type, before));
		assert(IsFullySupport(type, after));

		
		D3D12_RESOURCE_BARRIER_FLAGS native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;

		if(flags==BarrierFlags::BEGIN) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		if (flags == BarrierFlags::END) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;

		native.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
			static_cast<D3D12_RESOURCE_STATES>(before),
			static_cast<D3D12_RESOURCE_STATES>(after),
			subres,
			native_flags));

		validate();
	}




	void SubResourcesCPU::prepare_for(CommandListType type, SubResourcesGPU& state)
	{
		if (state.all_states_same && all_state.first_transition)
		{

			auto states = state.get_subres_state(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES).state;

			states = GetSupportedStates(type) & states;
			all_state.first_transition->wanted_state = merge_state(states, all_state.first_transition->wanted_state);
		}
	}

	ResourceStateManager::ResourceStateManager() : resource(static_cast<Resource*>(this))
	{
	}

	bool  ResourceStateManager::is_used(Transitions* list) const
	{
		SubResourcesCPU& s = get_state(list);
		return s.used;
	}

	SubResourcesCPU& ResourceStateManager::get_cpu_state(Transitions* list) const
	{
		auto& state = get_state(list);
		return state;
	}



	ResourceState ResourceStateManager::process_transitions(Barriers& target, std::vector<Resource*>& discards, Transitions* list) const
	{
		ResourceState processed_states = ResourceState::COMMON;

		if (!resource) return processed_states;

		auto& cpu_state = get_state(list);

		auto merge_one = [&, this](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	return;

			auto first_transition = cpu_state.get_first_transition(i);
			auto last_transition = cpu_state.get_last_transition(i);

			if (first_transition == last_transition) // if no transitions - merge ti! dont touch this, cmdlist is compiled
			{
				first_transition->wanted_state = merge_state(gpu.state, first_transition->wanted_state);
			}
		};



		auto transition_one = [&, this](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	return;

			auto first_transition = cpu_state.get_first_transition(i);
			auto last_transition = cpu_state.get_last_transition(i);

			auto first_state = first_transition->wanted_state;

			if (gpu.state != first_state)
			{
				target.transition(resource, gpu.state, first_state, i);
				processed_states = processed_states | gpu.state | first_state;
			}
		};


		if (cpu_state.all_state.first_transition && gpu_state.all_states_same)
		{
			merge_one(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			transition_one(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		}
		else
		{

			if (!cpu_state.all_state.used || (cpu_state.all_state.first_transition && cpu_state.all_states_same))
				for (int i = 0; i < gpu_state.subres.size(); i++)
				{
					merge_one(i);
				}
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				transition_one(i);
			}
		}

		gpu_state.set_cpu_state(cpu_state);


		if (cpu_state.need_discard)
		{
			discards.emplace_back(const_cast<Resource*>(resource));
		}

		return processed_states;
	}


	void ResourceStateManager::transition(Transitions* list, ResourceState state, unsigned int s) const
	{
		auto& cpu_state = get_state(list);

		Transition* last_transition = nullptr;

		bool need_add_uav = false;
		auto transition_one = [&](UINT subres, bool can_merge) {

			auto& subres_cpu = cpu_state.get_subres_state(subres);


			if (!subres_cpu.used)
			{
				subres_cpu.used = true;
				last_transition = subres_cpu.first_transition = subres_cpu.last_transition = list->create_transition(resource, subres, state);
			}
			else
			{
				auto last_state = subres_cpu.last_transition->wanted_state;// cpu_state.get_last_state(subres);


				if (last_state == state && state == ResourceState::UNORDERED_ACCESS)
				{
					need_add_uav = true;
				}
				else
				{
					auto merged_state = merge_state(last_state, state);
					bool was_merged = check(merged_state & subres_cpu.get_state());

					if (can_merge && was_merged)
					{
						subres_cpu.last_transition->wanted_state = merged_state;
					}
					else
					{
						auto transition = list->create_transition(resource, subres, state);
						last_transition = transition;
						subres_cpu.add_transition(transition);
					}
				}
			
			}
		};

		if (s == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			if (!cpu_state.all_states_same)
			{
				for (int i = 0; i < gpu_state.subres.size(); i++)
					transition_one(i, false);

				cpu_state.make_all_state(last_transition);
			}
			else
			{
				transition_one(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
			}
		}
		else
		{
			if (cpu_state.all_states_same)
			{
				cpu_state.make_unique_state();
				transition_one(s, false);
			}
			else
				transition_one(s, true);
		}


		if(need_add_uav)
		{
			list->create_uav_transition(resource);
		}
	}


	// optimization sector
	void ResourceStateManager::prepare_state(Transitions* from, SubResourcesGPU& gpu_state) const
	{

		//return ;
		auto& cpu_state = get_state(from);

		if (!cpu_state.used) return;

		bool updated = false;

		auto merge_one = [&, this](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	return;

			if (!IsFullySupport(from->get_type(), gpu.state))  return;

			auto first_transition = cpu_state.get_first_transition(i);
			auto last_transition = cpu_state.get_last_transition(i);

			first_transition->wanted_state = merge_state(gpu.state, first_transition->wanted_state);
		};




		auto transition_one = [&](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i, true);

			if (!cpu.used) return;

			if (!IsFullySupport(from->get_type(), gpu.state))  return;

			auto first_transition = cpu_state.get_first_transition(i);
			auto last_transition = cpu_state.get_last_transition(i);

			auto first_state = first_transition->wanted_state;

			if (gpu.state != first_state)
			{
				auto point = from->create_transition(resource, i, gpu.state, TransitionType::ZERO);
				cpu.set_zero_transition(point);
				updated = true;
			}
		};



		if (cpu_state.all_state.first_transition && gpu_state.all_states_same)
		{
			merge_one(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			transition_one(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		}
		else
		{

			if (!cpu_state.all_state.used)
				for (int i = 0; i < gpu_state.subres.size(); i++)
				{
					merge_one(i);
				}
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				transition_one(i);
			}
		}


		gpu_state.set_cpu_state(cpu_state);

		if (updated)
		{
			from->track_object(*resource);
			from->use_resource(resource);
		}

	}


	void ResourceStateManager::stop_using(Transitions* list, UINT subres) const
	{
		auto& state = get_state(list);

		
		auto transit = [&](UINT i)
		{
			auto last_transition = state.get_last_transition(i);

			last_transition->last_used_point = list->get_last_transition_point();
		};


		if (state.all_states_same)
		{
			transit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}
		
	}
	bool ResourceStateManager::transition(Transitions* from, Transitions* to) const
	{

		return false;
		auto& to_state = get_state(to);

		if (!to_state.used)
			return false;


		bool used = false;

		auto check = [&](UINT i)
		{
			auto& subres_to = to_state.get_subres_state(i);
			if (subres_to.used)
			{
				if (IsFullySupport(from->get_type(), subres_to.get_first_state()))

					used = true;
				else
					return false;
			}

			return true;
		};


		if (to_state.all_states_same)
			if (!check(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)) return false;
			else
			{
				for (int i = 0; i < gpu_state.subres.size(); i++)
					if (!check(i))  return false;
			}

		if (!used) return false;

		auto transit = [&](UINT i)
		{
			auto& subres_to = to_state.get_subres_state(i);

			if (subres_to.used)
				transition(from, subres_to.get_first_state(), i);
		};


		if (to_state.all_states_same)
		{
			transit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}

		return true;
	}

}
