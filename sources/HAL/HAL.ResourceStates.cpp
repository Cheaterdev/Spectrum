module HAL:ResourceStates;
import <HAL.h>;
import Core;

import HAL;
namespace HAL
{
		 void ResourceListStateCPU::check_valid(const Resource* resource)
			{
			   if(resource&&check(resource->get_desc().Flags & ResFlags::DisableStateTracking))	return;



				auto usage = last_usage;
				auto prev = last_usage->prev_usage;
				if(prev)
				{
//					ASSERT(!(prev->wanted_state.operation==BarrierSync::NONE) );
				
				}
			}


	void Barriers::validate()
	{

#ifdef _DEV
		for (int j = 0; j < native.size() - 1; j++)
		{
			if (native.back().Type == native[j].Type)
				if (native.back().ResourceUsage.pResource == native[j].ResourceUsage.pResource)
					if (native.back().ResourceUsage.Subresource == native[j].ResourceUsage.Subresource)
						ASSERT(false);
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


	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		ASSERT(resource);

		ASSERT(IsFullySupport(type, before));
		ASSERT(IsFullySupport(type, after));

		if (check(after.access & (BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ)))
			ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));

		barriers.emplace_back(Barrier{ const_cast<Resource*>(resource) ,before ,after ,subres ,flags });//

		validate();
	}




	void SubResourcesCPU::merge_read_state(CommandListType type, SubResourcesGPU& state)
	{
		if (!used) return;

		for (int i = 0; i < subres.size(); i++)
		{
			auto my_state = get_subres_state(i);
			auto subres_layout = state.get_subres_state(i).layout;

			if (subres_layout == TextureLayout::NONE)
				continue;

			auto merged = merge_layout(subres_layout, my_state.first_usage->wanted_state.layout);

			if (merged)
				my_state.first_usage->wanted_state.layout = *merged;
			else
				ASSERT(false);

			 my_state.check_valid(nullptr);
			ASSERT(my_state.first_usage == my_state.last_usage);
		}
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



#ifdef PRETRANSITIONS_FIX
	CommandListType ResourceStateManager::process_transitions(Barriers& target, Transitions* list) const
	{
		CommandListType cmd_type = CommandListType::COPY;
		if (!resource) return cmd_type;

		ASSERT(!check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		if (resource->get_desc().is_buffer())
			return cmd_type;

		auto& cpu_state = get_state((list));



		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	continue;

			auto first_usage = cpu_state.get_first_usage(i);
			//	auto last_usage = cpu_state.get_last_usage(i);

			auto first_state = first_usage->wanted_state;

			if (first_state != ResourceStates::UNKNOWN)
				if (gpu.layout != first_state.layout)
				{

			
					auto from = ResourceStates::NO_ACCESS;
					//auto to = ResourceStates::NO_ACCESS;

					from.layout = gpu.layout;

					//	to.layout = first_state.layout;


						//ASSERT(! first_state.layout!=);
					target.transition(resource, from, first_state, i);

					cmd_type = Merge(cmd_type, from.get_best_cmd_type());
					cmd_type = Merge(cmd_type, first_state.get_best_cmd_type());

				}

				 cpu.check_valid(resource);
		}

		gpu_state.set_cpu_state(cpu_state);

		return cmd_type;
	}
#endif

	void ResourceStateManager::transition(Transitions* list, ResourceState state, unsigned int s) const
	{
		if (check(state.access & (BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ)))
			ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));



		auto& cpu_state = get_state((list));

		//ResourceUsage* last_usage = nullptr;

		bool need_add_uav = false;

		if (list->get_type() == CommandListType::COPY)
		{
			if (state.layout == TextureLayout::COPY_DEST || state.layout == TextureLayout::COPY_SOURCE)
			{
				state.layout = TextureLayout::COPY_QUEUE;

			}
		}
		ASSERT(state.is_valid(resource->get_type()));
		auto transition_one = [&](UINT subres) {

			auto& subres_cpu = cpu_state.get_subres_state(subres);
			if (!subres_cpu.used)
			{
				if(!check(resource->get_desc().Flags & ResFlags::DisableStateTracking)&&resource->get_desc().is_texture())
				{			 
					auto target = ResourceStates::NO_ACCESS;
					target.layout = initial_layout;

					subres_cpu.used = true;
					subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, target);


				 //  if(target.layout!=state.layout)
				   {
				   		auto transition = (list)->add_usage((resource), subres, state);
						subres_cpu.add_usage(transition);			   
				   }
				}else	 
				{
					subres_cpu.used = true;
					subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, state);			
				
				}
				 subres_cpu.check_valid(resource);
			  	return;
			}


			{
				auto last_state = subres_cpu.last_usage->wanted_state;

			//	if(last_state!=state)
				{
					auto merged_state = merge_state(last_state, state);
					if (merged_state)
					{
						subres_cpu.last_usage->wanted_state = *merged_state;
					}
					else
					{
						auto transition = (list)->add_usage((resource), subres, state);
						subres_cpu.add_usage(transition);
					}

					 subres_cpu.check_valid(resource);
				}

			}

			};

		for (int i = 0; i < gpu_state.subres.size(); i++)
			transition_one(i);
	}


	// optimization sector
	void ResourceStateManager::prepare_state(Transitions* from, const SubResourcesGPU& gpu_state) const
	{
		if (resource->get_desc().is_buffer())
			return;

	   ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		//return ;
		auto& cpu_state = get_state((from));

		if (!cpu_state.used) return;

		bool updated = false;

		auto merge_one = [&, this](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	return;

			//	ASSERT(IsFullySupport((from)->get_type(), gpu.state));

			auto first_usage = cpu_state.get_first_usage(i);


			auto merged = merge_layout(gpu.layout, first_usage->wanted_state.layout);
			if (merged)
				first_usage->wanted_state.layout = *merged;
			ASSERT(first_usage->wanted_state.valid_begin());

			ASSERT(first_usage->wanted_state.valid_begin());

			 cpu.check_valid(resource);
			};




		auto transition_one = [&](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used) return;

			//ASSERT(IsFullySupport((from)->get_type(), gpu.state));

			auto first_usage = cpu_state.get_first_usage(i);


			auto first_state = first_usage->wanted_state;

			if (gpu.layout != first_state.layout)
			{
				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu.layout;

				auto point = (from)->add_usage((resource), i, target, TransitionType::ZERO);
				cpu.set_zero_transition(point);
				updated = true;

				 cpu.check_valid(resource);
			}
			};



		/*	if (cpu_state.all_state.first_usage && gpu_state.all_states_same)
			{
				merge_one(ALL_SUBRESOURCES);
				transition_one(ALL_SUBRESOURCES);
			}
			else*/
		{

			//	if (!cpu_state.all_state.used)
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				merge_one(i);
			}
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				transition_one(i);
			}
		}


		//	gpu_state.set_cpu_state(cpu_state);

		if (updated)
		{
			(from)->track_object(*(const_cast<Resource*>(resource)));
			(from)->use_resource((resource));
		}

	}

	void ResourceStateManager::prepare_state(Transitions* from, ResourceState wanted_state) const
	{
		if (resource->get_desc().is_buffer())
			return;

	   ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		//return ;
		auto& cpu_state = get_state((from));

		//		if (!cpu_state.used) return;

		bool updated = false;

		auto merge_one = [&, this](UINT i) {
			auto& gpu = wanted_state;//gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)	return;

			//	ASSERT(IsFullySupport((from)->get_type(), gpu.state));

			auto first_usage = cpu_state.get_first_usage(i);


			auto merged = merge_layout(gpu.layout, first_usage->wanted_state.layout);
			if (merged)
				first_usage->wanted_state.layout = *merged;
			ASSERT(first_usage->wanted_state.valid_begin());

			ASSERT(first_usage->wanted_state.valid_begin());

			 cpu.check_valid(resource);
			};




		auto transition_one = [&](UINT i) {
			auto& gpu = wanted_state;//gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used)
			{
				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu.layout;

				auto point = (from)->add_usage((resource), i, target, TransitionType::ZERO);
				cpu.set_zero_transition(point);
				updated = true;

				cpu.used = true;

				 cpu.check_valid(resource);
				return;
			}

			//ASSERT(IsFullySupport((from)->get_type(), gpu.state));

			auto first_usage = cpu_state.get_first_usage(i);


			auto first_state = first_usage->wanted_state;

			if (gpu.layout != first_state.layout)
			{
				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu.layout;

				auto point = (from)->add_usage((resource), i, target, TransitionType::ZERO);
				cpu.set_zero_transition(point);
				updated = true;
			}

			 cpu.check_valid(resource);
			};


			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				merge_one(i);
			}
			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				transition_one(i);
			}
		

			
		if (updated)
		{
			(from)->track_object(*(const_cast<Resource*>(resource)));
			(from)->use_resource((resource));
		}

	}
	void ResourceStateManager::alias_begin(Transitions* list) const
	{
		transition(list, ResourceStates::UNKNOWN, ALL_SUBRESOURCES);
	}



	void ResourceStateManager::alias_end(Transitions* list) const
	{
		transition(list, ResourceStates::UNKNOWN, ALL_SUBRESOURCES);
	}


	void ResourceStateManager::prepare_after_state(Transitions* from, const SubResourcesGPU& gpu_state) const
	{
		if (resource->get_desc().is_buffer())
			return;
			
		ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		auto& cpu_state = get_state((from));

		{

			for (int i = 0; i < gpu_state.subres.size(); i++)
			{
				if (gpu_state.subres[i].layout == TextureLayout::NONE) continue;

				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu_state.subres[i].layout;
				transition(from, target, i);
			}
		}

		if (!cpu_state.used)
		{
			(from)->track_object(*(const_cast<Resource*>(resource)));
			(from)->use_resource((resource));
		}
	}


	void ResourceStateManager::stop_using(Transitions* list, UINT subres) const
	{
		auto& state = get_state((list));


		auto transit = [&](UINT i)
			{
				auto last_usage = state.get_last_usage(i);

				last_usage->last_point = (list)->get_last_usage_point();
			};

		if (state.used)
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}	 

	}

	void ResourceStateManager::connect(Transitions* from, Transitions* to)
	{
		if (resource->get_desc().is_buffer())
			return;

		ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		auto& prev_state = get_state((from));
		auto& next_state = get_state((to));

		auto transit = [&](UINT i)
			{
				auto last_usage = prev_state.get_last_usage(i);
				auto first_usage = next_state.get_first_usage(i);

				ASSERT(!first_usage->prev_usage);
				ASSERT(last_usage->point->cmd_list != first_usage->point->cmd_list);

				if (first_usage->wanted_state != last_usage->wanted_state)
				{
					first_usage->prev_usage = last_usage;


					first_usage->debug = true;
				}
			};


		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}
	}
}
