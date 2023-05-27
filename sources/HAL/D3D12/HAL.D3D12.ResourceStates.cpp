module HAL:ResourceStates;

import Core;

import HAL;
namespace HAL
{


	void Barriers::validate()
	{

#ifdef _DEV
		for (int j = 0; j < native.size() - 1; j++)
		{
			if (native.back().Type == native[j].Type)
				if (native.back().ResourceUsage.pResource == native[j].ResourceUsage.pResource)
					if (native.back().ResourceUsage.Subresource == native[j].ResourceUsage.Subresource)
						assert(false);
		}

#endif
	}
	Barriers::Barriers(CommandListType type) :type(type)
	{

	}
	void Barriers::clear()
	{
		barriers.clear();
	}
	const std::vector<Barrier>& Barriers::get_barriers() const
	{
		return barriers;
	}
	//void Barriers::uav(Resource* resource)
	//{
	//	barriers.emplace_back(BarrierUAV{ resource });// CD3DX12_RESOURCE_BARRIER::UAV((resource)->get_dx()));
	//}

	//void Barriers::alias(Resource* from, Resource* to)
	//{
	//	barriers.emplace_back(BarrierAlias{ from, to });// (CD3DX12_RESOURCE_BARRIER::Aliasing(native_from, native_to));
	//}

	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		//if (before == ResourceStates::RAYTRACING_STRUCTURE) return; // TODO: make proper target state for raytrace
		//if (after == ResourceStates::RAYTRACING_STRUCTURE) return; // TODO: make proper target state for raytrace
		assert(resource);

	//	assert(before != ResourceStates::UNKNOWN);
	//	assert(after != ResourceStates::UNKNOWN);

		assert(IsFullySupport(type, before));
		assert(IsFullySupport(type, after));

		barriers.emplace_back(Barrier{ const_cast<Resource*>(resource) ,before ,after ,subres ,flags });//
		//D3D12_RESOURCE_BARRIER_FLAGS native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;

		//if (flags == BarrierFlags::BEGIN) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		//if (flags == BarrierFlags::END) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;

		//native.emplace_back(CD3DX12_RESOURCE_BARRIER::ResourceUsage((resource)->get_dx(),
		//	static_cast<D3D12_RESOURCE_STATES>(before),
		//	static_cast<D3D12_RESOURCE_STATES>(after),
		//	subres== ALL_SUBRESOURCES? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES:subres,
		//	native_flags));

		validate();
	}




	void SubResourcesCPU::prepare_for(CommandListType type, SubResourcesGPU& state)
	{
		//if(!used) return;

		//if (state.all_states_same && all_state.first_usage)
		//{

		//	auto states = state.get_subres_state(ALL_SUBRESOURCES).state;

		//	//states = GetSupportedStates(type) & states;
		//	all_state.first_usage->wanted_state = merge_state(states, all_state.first_usage->wanted_state);
		//}else
		//{
		//
		//	make_unique_state();


		//		for (int i = 0; i < subres.size(); i++)
		//			{
		//			auto my_state = get_subres_state(i);
		//				auto subres_state = state.get_subres_state(i).state;

		//				if(subres_state==ResourceState::UNKNOWN) 
		//					continue;

		//				my_state.first_usage->wanted_state = merge_state(subres_state, my_state.first_usage->wanted_state);
		//				
		//			}

		//}
	}

	ResourceStateManager::ResourceStateManager(const Resource* resource) : resource(resource)
	{
	}

	bool  ResourceStateManager::is_used(Transitions* list) const
	{
		SubResourcesCPU& s = get_state((list));
		return s.used;
	}

	SubResourcesCPU& ResourceStateManager::get_cpu_state(Transitions* list) const
	{
		auto& state = get_state((list));
		return state;
	}



	ResourceState ResourceStateManager::process_transitions(Barriers& target, std::vector<Resource*>& discards, Transitions* list) const
	{
		ResourceState processed_states = ResourceStates::PIXEL_SHADER_RESOURCE;

		if (!resource) return processed_states;

		auto& cpu_state = get_state((list));



					for (int i = 0; i < gpu_state.subres.size(); i++)
				{
						auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	continue;

			auto first_usage = cpu_state.get_first_usage(i);
		//	auto last_usage = cpu_state.get_last_usage(i);

			auto first_state = first_usage->wanted_state;

			if (gpu.state != first_state)
			{
				//assert(!manual_controlled);
				target.transition(resource, gpu.state, first_state, i);
				processed_states = processed_states | gpu.state | first_state;
			}
				}
		 
		// 
		// 
		//auto merge_one = [&, this](UINT i) {
		//	auto& gpu = gpu_state.get_subres_state(i);
		//	auto& cpu = cpu_state.get_subres_state(i);

		//	if (!cpu.used)	return;

		//	auto first_usage = cpu_state.get_first_usage(i);
		//	auto last_usage = cpu_state.get_last_usage(i);

		//	if (first_usage == last_usage) // if no transitions - merge ti! dont touch this, cmdlist is compiled
		//	{
		//		first_usage->wanted_state = merge_state(gpu.state, first_usage->wanted_state);
		//	}
		//};



		//auto transition_one = [&, this](UINT i) {
		//	auto& gpu = gpu_state.get_subres_state(i);
		//	auto& cpu = cpu_state.get_subres_state(i);

		//	if (!cpu.used)	return;

		//	auto first_usage = cpu_state.get_first_usage(i);
		//	auto last_usage = cpu_state.get_last_usage(i);

		//	auto first_state = first_usage->wanted_state;

		//	if (gpu.state != first_state)
		//	{
		//		//assert(!manual_controlled);
		//		target.transition(resource, gpu.state, first_state, i);
		//		processed_states = processed_states | gpu.state | first_state;
		//	}
		//};


		//if (cpu_state.all_state.first_usage && gpu_state.all_states_same)
		//{
		//	merge_one(ALL_SUBRESOURCES);
		//	transition_one(ALL_SUBRESOURCES);
		//}
		//else
		//{

		//	if (!cpu_state.all_state.used || (cpu_state.all_state.first_usage && cpu_state.all_states_same))
		//		for (int i = 0; i < gpu_state.subres.size(); i++)
		//		{
		//			merge_one(i);
		//		}
		//	for (int i = 0; i < gpu_state.subres.size(); i++)
		//	{
		//		transition_one(i);
		//	}
		//}

		gpu_state.set_cpu_state(cpu_state);


		//if (cpu_state.need_discard)
		//{
		//	discards.emplace_back(const_cast<Resource*>(resource));
		//}

		return processed_states;
	}


	void ResourceStateManager::transition(Transitions* list, ResourceState state, unsigned int s) const
	{
		auto& cpu_state = get_state((list));

		ResourceUsage* last_usage = nullptr;

		bool need_add_uav = false;
		auto transition_one = [&](UINT subres) {

			auto& subres_cpu = cpu_state.get_subres_state(subres);


			if (!subres_cpu.used)
			{
				subres_cpu.used = true;
				last_usage = subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, state);
			}
			else
			{
				auto last_state = subres_cpu.last_usage->wanted_state;// cpu_state.get_last_state(subres);


			/*	if (last_state == state && state == ResourceStates::UNORDERED_ACCESS)
				{
					need_add_uav = true;
				}
				else*/
				{
					auto merged_state = merge_state(last_state, state);
					//bool was_merged = check(merged_state & subres_cpu.get_usage());

					if (merged_state)
					{
						subres_cpu.last_usage->wanted_state = *merged_state;
					}
					else
					{
						auto transition = (list)->add_usage((resource), subres, state);
						last_usage = transition;
						subres_cpu.add_usage(transition);
					}
				}

			}
		};

			for (int i = 0; i < gpu_state.subres.size(); i++)
					transition_one(i);

		/*if (need_add_uav)
		{
			(list)->create_uav_transition((resource));
		}*/
	}


	// optimization sector
	void ResourceStateManager::prepare_state(Transitions* from, SubResourcesGPU& gpu_state) const
	{

		////return ;
		//auto& cpu_state = get_state((from));

		//if (!cpu_state.used) return;

		//bool updated = false;

		//auto merge_one = [&, this](UINT i) {
		//	auto& gpu = gpu_state.get_subres_state(i);
		//	auto& cpu = cpu_state.get_subres_state(i);

		//	if (!cpu.used)	return;

		//	assert (IsFullySupport((from)->get_type(), gpu.state));

		//	auto first_usage = cpu_state.get_first_usage(i);
		//	auto last_usage = cpu_state.get_last_usage(i);

		//	first_usage->wanted_state = merge_state(gpu.state, first_usage->wanted_state);
		//};




		//auto transition_one = [&](UINT i) {
		//	auto& gpu = gpu_state.get_subres_state(i);
		//	auto& cpu = cpu_state.get_subres_state(i, true);

		//	if (!cpu.used) return;

		//	assert (IsFullySupport((from)->get_type(), gpu.state));

		//	auto first_usage = cpu_state.get_first_usage(i);
		//	auto last_usage = cpu_state.get_last_usage(i);

		//	auto first_state = first_usage->wanted_state;

		//	if (gpu.state != first_state)
		//	{
		//		auto point = (from)->create_transition((resource), i, gpu.state, TransitionType::ZERO);
		//		//		cpu.set_zero_transition(point);
		//		updated = true;
		//	}
		//};



		//if (cpu_state.all_state.first_usage && gpu_state.all_states_same)
		//{
		//	merge_one(ALL_SUBRESOURCES);
		//	transition_one(ALL_SUBRESOURCES);
		//}
		//else
		//{

		//	if (!cpu_state.all_state.used)
		//		for (int i = 0; i < gpu_state.subres.size(); i++)
		//		{
		//			merge_one(i);
		//		}
		//	for (int i = 0; i < gpu_state.subres.size(); i++)
		//	{
		//		transition_one(i);
		//	}
		//}


		//gpu_state.set_cpu_state(cpu_state);

		//if (updated)
		//{
		//	(from)->track_object(*(const_cast<Resource*>(resource)));
		//	(from)->use_resource((resource));
		//}

	}


		void ResourceStateManager::prepare_after_state(Transitions* from, SubResourcesGPU& gpu_state) const
	{

//		//return ;
//		auto& cpu_state = get_state((from));
//
//	//	if (!cpu_state.used)
//	//		return;
//
//		bool updated = false;
//
//
//		if ( gpu_state.all_states_same)
//		{
//			assert (IsFullySupport((from)->get_type(), gpu_state.all_state.state));
//				transition(from,gpu_state.all_state.state,ALL_SUBRESOURCES) ;
//		}
//		else
//		{
//
//	/*		if (!cpu_state.all_state.used)
//				for (int i = 0; i < gpu_state.subres.size(); i++)
//				{
//					merge_one(i);
//				}*/
//			for (int i = 0; i < gpu_state.subres.size(); i++)
//			{
//				if(gpu_state.subres[i].state==ResourceState::UNKNOWN) continue;
//				assert (IsFullySupport((from)->get_type(), gpu_state.subres[i].state));
//			transition(from,gpu_state.subres[i].state,i) ;
//			}
//		}
//
//
////		gpu_state.set_cpu_state(cpu_state);
//
//		if (updated)
//		{
//			(from)->track_object(*(const_cast<Resource*>(resource)));
//			(from)->use_resource((resource));
//		}

	}


	//void ResourceStateManager::stop_using(Transitions* list, UINT subres) const
	//{
	//	/*auto& state = get_state((list));


	//	auto transit = [&](UINT i)
	//	{
	//		auto last_usage = state.get_last_usage(i);

	//		last_usage->last_used_point = (list)->get_last_usage_point();
	//	};


	//	if (state.all_states_same)
	//	{
	//		transit(ALL_SUBRESOURCES);
	//	}
	//	else
	//	{
	//		for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
	//	}*/

	//}
	bool ResourceStateManager::transition(Transitions* from, Transitions* to) const
	{

		return false;
		/*auto& to_state = get_state((to));

		if (!to_state.used)
			return false;


		bool used = false;

		auto check = [&](UINT i)
		{
			auto& subres_to = to_state.get_subres_state(i);
			if (subres_to.used)
			{
				if (IsFullySupport((from)->get_type(), subres_to.get_first_state()))

					used = true;
				else
					return false;
			}

			return true;
		};


		if (to_state.all_states_same)
			if (!check(ALL_SUBRESOURCES)) return false;
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
			transit(ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}

		return true;*/
	}

}
