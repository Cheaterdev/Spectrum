module HAL:ResourceStates;

import Core;

import HAL;
namespace HAL
{

	// --- free functions ---

	bool IsCompatible(CommandListType a, CommandListType b)
	{
		if (a == CommandListType::DIRECT) return true;
		if (b == CommandListType::DIRECT) return false;

		if (a == CommandListType::COMPUTE) return true;
		if (b == CommandListType::COMPUTE) return false;

		return true;
	}

	bool IsFullySupport(CommandListType type, const ResourceState& states)
	{
		return IsCompatible(type, states.get_best_cmd_type());
	}

	CommandListType Merge(CommandListType a, CommandListType b)
	{
		if (a == CommandListType::DIRECT || b == CommandListType::DIRECT) return CommandListType::DIRECT;
		if (a == CommandListType::COMPUTE || b == CommandListType::COMPUTE) return CommandListType::COMPUTE;
		return CommandListType::COPY;
	}

	std::optional<ResourceState> merge_state(const ResourceState& source, const ResourceState& need)
	{
		if (source == ResourceStates::UNKNOWN)
			return std::nullopt;

		if (source.has_write_bits() || need.has_write_bits())
			return std::nullopt;

		if (source.is_no_access() || need.is_no_access())
			return std::nullopt;

		auto res = source | need;
		return res;
	}

	std::optional<TextureLayout> merge_layout(const TextureLayout& source, const TextureLayout& need)
	{
		if (source == TextureLayout::NONE) return need;
		if (source == need) return need;
		if (source == TextureLayout::UNDEFINED) return need;

		if (need == TextureLayout::UNDEFINED) return need;		  // for alias end

		static const TextureLayout LAYOUT_WRITE = TextureLayout::UNORDERED_ACCESS | TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::RENDER_TARGET | TextureLayout::COPY_DEST;
		if (check(source & LAYOUT_WRITE) || check(need & LAYOUT_WRITE))
			return std::nullopt;

		return source | need;
	}


	// --- Barriers ---

	Barriers::Barriers(CommandListType type) : type(type)
	{
	}

	Barriers::operator bool() const
	{
		return !barriers.empty();
	}

	void Barriers::clear()
	{
		barriers.clear();
	}

	const std::vector<Barrier>& Barriers::get_barriers() const
	{
		return barriers;
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

	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		ASSERT(resource);

		ASSERT(IsFullySupport(type, before));
		ASSERT(IsFullySupport(type, after));

		if (check(after.access & (BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ)))
			ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));

		barriers.emplace_back(Barrier{ const_cast<Resource*>(resource), before, after, subres, flags });

		validate();
	}


	// --- UsagePoint ---

	UsagePoint::UsagePoint(CommandListType type) : transitions(type)
	{
	}


	// --- ResourceListStateCPU ---

	ResourceState ResourceListStateCPU::get_first_usage()
	{
		return first_usage->wanted_state;
	}

	ResourceState ResourceListStateCPU::get_usage()
	{
		return last_usage->wanted_state;
	}

	void ResourceListStateCPU::reset()
	{
		need_discard = false;
		used = false;
		first_usage = nullptr;
		last_usage = nullptr;
	}

	void ResourceListStateCPU::check_valid(const Resource* resource)
	{
		if (resource && check(resource->get_desc().Flags & ResFlags::DisableStateTracking)) return;

		auto usage = last_usage;
		auto prev_usage = last_usage->prev_usage;

		if(prev_usage && prev_usage->prev_usage && prev_usage->wanted_state!=ResourceStates::UNKNOWN)
			ASSERT( prev_usage->wanted_state.operation!=BarrierSync::NONE);


	}

	ResourceUsage* ResourceListStateCPU::add_usage(ResourceUsage* usage)
	{
	//	ASSERT(!last_usage || !(usage->wanted_state == last_usage->wanted_state && usage->wanted_state.access == ResourceStates::NO_ACCESS.access));
	//	ASSERT(!last_usage || (last_usage->wanted_state.operation != ResourceStates::NO_ACCESS.operation));
		
	//	ASSERT(!(!check(usage->resource->get_desc().Flags & ResFlags::DisableStateTracking) && usage->wanted_state.access == ResourceStates::NO_ACCESS.access));
	   ASSERT(!(check(usage->resource->get_desc().Flags & ResFlags::DisableStateTracking) && last_usage &&last_usage->prev_usage && last_usage->wanted_state.access == ResourceStates::NO_ACCESS.access));
		
		auto prev = last_usage;
		last_usage = usage;
		last_usage->prev_usage = prev;
		if (prev) prev->next_usage = usage;

		return usage;
	}

	ResourceUsage* ResourceListStateCPU::set_zero_transition(ResourceUsage* usage)
	{
	//		ASSERT(!(!check(usage->resource->get_desc().Flags & ResFlags::DisableStateTracking) && usage->wanted_state.access == ResourceStates::NO_ACCESS.access));
	
		usage->next_usage = first_usage;
		if (first_usage) first_usage->prev_usage = usage;

		if (!last_usage) last_usage = usage;

		first_usage = usage;
		return first_usage;
	}


	// --- SubResourcesCPU ---

	void SubResourcesCPU::reset()
	{
		used = false;
		for (auto& s : subres)
			s.reset();
	}

	CommandListType SubResourcesCPU::get_best_list_type_last()
	{
		CommandListType type = CommandListType::COPY;

		for (auto& cpu : subres)
		{
			if (!cpu.used) continue;
			auto state = cpu.last_usage->wanted_state;
			type = Merge(type, state.get_best_cmd_type());
		}

		return type;
	}

	CommandListType SubResourcesCPU::get_best_list_type_first()
	{
		CommandListType type = CommandListType::COPY;

		for (auto& cpu : subres)
		{
			if (!cpu.used) continue;
			auto state = cpu.first_usage->wanted_state;
			type = Merge(type, state.get_best_cmd_type());
		}

		return type;
	}

	const ResourceListStateCPU& SubResourcesCPU::get_subres_state(UINT id) const
	{
		return subres[id];
	}

	ResourceListStateCPU& SubResourcesCPU::get_subres_state(UINT id)
	{
		return subres[id];
	}

	ResourceUsage* SubResourcesCPU::get_first_usage(UINT id) const
	{
		return subres[id].first_usage;
	}

	ResourceUsage* SubResourcesCPU::get_last_usage(UINT id) const
	{
		return subres[id].last_usage;
	}

	ResourceState SubResourcesCPU::get_first_state(UINT id) const
	{
		return get_first_usage(id)->wanted_state;
	}

	ResourceState SubResourcesCPU::get_last_state(UINT id) const
	{
		return get_last_usage(id)->wanted_state;
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


	// --- SubResourcesGPU ---

	bool SubResourcesGPU::is_valid() const
	{
		for (auto& s : subres)
			return true;
		return false;
	}

	void SubResourcesGPU::operator=(const TextureLayout& layout)
	{
		for (auto& s : subres)
			s.layout = layout;
	}

	CommandListType SubResourcesGPU::get_best_list_type()
	{
		CommandListType type = CommandListType::COPY;

		for (auto& s : subres)
		{
			if (s.layout != TextureLayout::NONE)
				type = Merge(type, get_best_cmd_type(s.layout));
		}

		return type;
	}

	void SubResourcesGPU::set_cpu_state(const SubResourcesCPU& cpu_state)
	{
		for (int i = 0; i < subres.size(); i++)
		{
			auto& gpu = get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used) continue;

			auto last_usage = cpu_state.get_last_usage(i);
			gpu.layout = last_usage->wanted_state.layout;
		}
	}

	void SubResourcesGPU::set_cpu_state_first(const SubResourcesCPU& cpu_state)
	{
		for (int i = 0; i < subres.size(); i++)
		{
			auto& gpu = get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used) continue;

			auto first_usage = cpu_state.get_first_usage(i);
			gpu.layout = first_usage->wanted_state.layout;
		}
	}

	const ResourceListStateGPU& SubResourcesGPU::get_subres_state(UINT id) const
	{
		return subres[id];
	}

	ResourceListStateGPU& SubResourcesGPU::get_subres_state(UINT id)
	{
		return subres[id];
	}

	void SubResourcesGPU::merge(SubResourcesCPU& other)
	{
		for (int i = 0; i < subres.size(); i++)
		{
			auto transition = other.get_first_usage(i);

			if (transition)
			{
				auto res = merge_layout(subres[i].layout, transition->wanted_state.layout);
				if (res)
					subres[i].layout = *res;
			}
		}
	}


	// --- ResourceStateManager ---

	ResourceStateManager::ResourceStateManager(const Resource* resource) : resource(resource)
	{
	}

	UINT ResourceStateManager::get_subres_count()
	{
		return static_cast<UINT>(gpu_state.subres.size());
	}

	SubResourcesGPU ResourceStateManager::copy_gpu() const
	{
		return gpu_state;
	}

	void ResourceStateManager::init_subres(int count, TextureLayout layout)
	{
		initial_layout = layout;
		gpu_state.subres.resize(count);
		for (auto& e : gpu_state.subres)
			e.layout = layout;

		// Reset every per-command-list CPU state so that set_cpu_state_first() on
		// the next barrier-compilation pass sees "never used" and falls back to the
		// GPU state we just reset above, rather than overwriting it with a stale
		// layout from a previous frame (e.g. COLOR_ATTACHMENT_OPTIMAL after present).
		states.set_init_func([count](SubResourcesCPU& state)
			{
				state.used = false;
				state.subres.resize(count);
				for (auto& e : state.subres)
					e.used = false;   // prevents set_cpu_state_first from using stale layout
			});
	}

	bool ResourceStateManager::is_used(Transitions* list) const
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

			if (!cpu.used) continue;

			auto first_usage = cpu_state.get_first_usage(i);
			auto first_state = first_usage->wanted_state;

			if (first_state != ResourceStates::UNKNOWN)
				if (gpu.layout != first_state.layout)
				{
					auto from = ResourceStates::NO_ACCESS;
					from.layout = gpu.layout;

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

		if (check(resource->get_desc().Flags & ResFlags::Immutable))
		{
				  		ASSERT(!state.has_write_bits() || state.has_copy_bits());

						ASSERT(state.access != ResourceStates::NO_ACCESS.access);
		}
	

		auto& cpu_state = get_state((list));

		bool need_add_uav = false;

		if (list->get_type() == CommandListType::COPY)
		{
			if (state.layout == TextureLayout::COPY_DEST || state.layout == TextureLayout::COPY_SOURCE)
				state.layout = TextureLayout::COPY_QUEUE;
		}

		ASSERT(state.is_valid(resource->get_type()));

		auto transition_one = [&](UINT subres) {

			auto& subres_cpu = cpu_state.get_subres_state(subres);
			if (!subres_cpu.used)
			{
				if (!check(resource->get_desc().Flags & ResFlags::DisableStateTracking) && resource->get_desc().is_texture())
				{
					auto target = ResourceStates::NO_ACCESS;
					target.layout = initial_layout;

					subres_cpu.used = true;
					subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, target);

					//	ASSERT(!(!check(subres_cpu.first_usage->resource->get_desc().Flags & ResFlags::DisableStateTracking) && subres_cpu.first_usage->wanted_state.access == ResourceStates::NO_ACCESS.access));
	
					{
						auto transition = (list)->add_usage((resource), subres, state);
						subres_cpu.add_usage(transition);
					}
				}
				else
				{
					subres_cpu.used = true;
					subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, state);
				}
				subres_cpu.check_valid(resource);
				return;
			}

			{
				auto last_state = subres_cpu.last_usage->wanted_state;

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


		if(s!=ALL_SUBRESOURCES)
		{
			transition_one(s);
		}
		else
		for (int i = 0; i < gpu_state.subres.size(); i++)
			transition_one(i);
	}


	// optimization sector
	void ResourceStateManager::prepare_state(Transitions* next_list, const SubResourcesGPU& gpu_state) const
	{
		if (resource->get_desc().is_buffer())
			return;

		ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		auto& cpu_state = get_state((next_list));

		//if (!cpu_state.used) return;

		bool updated = false;

		auto merge_one = [&, this](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used) return;

			auto first_usage = cpu_state.get_first_usage(i);

			auto merged = merge_layout(gpu.layout, first_usage->wanted_state.layout);
			if (merged)
				first_usage->wanted_state.layout = *merged;
	//		ASSERT(first_usage->wanted_state.valid_begin());
		//	ASSERT(first_usage->wanted_state.valid_begin());

			cpu.check_valid(resource);
		};

		auto transition_one = [&](UINT i) {
			auto& gpu = gpu_state.get_subres_state(i);
			auto& cpu = cpu_state.get_subres_state(i);				

			auto first_usage = cpu_state.get_first_usage(i);
		
			if (!cpu.used || gpu.layout != first_usage->wanted_state.layout)
			{
				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu.layout;

				auto point = (next_list)->add_usage((resource), i, target, TransitionType::ZERO);
				cpu.set_zero_transition(point);
				updated = true;
					cpu.used = true;
				cpu.check_valid(resource);
			}
		};

		for (int i = 0; i < gpu_state.subres.size(); i++) merge_one(i);
		for (int i = 0; i < gpu_state.subres.size(); i++) transition_one(i);

		if (updated)
		{
			(next_list)->track_object(*(const_cast<Resource*>(resource)));
			(next_list)->use_resource((resource));
		}
	}

	void ResourceStateManager::prepare_state(Transitions* from, ResourceState wanted_state) const
	{
		if (resource->get_desc().is_buffer())
			return;

		ASSERT(check(resource->get_desc().Flags & ResFlags::DisableStateTracking));

		auto& cpu_state = get_state((from));

		bool updated = false;

		auto merge_one = [&, this](UINT i) {
			auto& gpu = wanted_state;
			auto& cpu = cpu_state.get_subres_state(i);

			if (!cpu.used) return;

			auto first_usage = cpu_state.get_first_usage(i);

			auto merged = merge_layout(gpu.layout, first_usage->wanted_state.layout);
			if (merged)
				first_usage->wanted_state.layout = *merged;
			ASSERT(first_usage->wanted_state.valid_begin());
			ASSERT(first_usage->wanted_state.valid_begin());

			cpu.check_valid(resource);
		};

		auto transition_one = [&](UINT i) {
			auto& gpu = wanted_state;
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

		for (int i = 0; i < gpu_state.subres.size(); i++) merge_one(i);
		for (int i = 0; i < gpu_state.subres.size(); i++) transition_one(i);

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

		for (int i = 0; i < gpu_state.subres.size(); i++)
		{
			if (gpu_state.subres[i].layout == TextureLayout::NONE) continue;

			auto target = ResourceStates::NO_ACCESS;
			target.layout = gpu_state.subres[i].layout;
			transition(from, target, i);
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
			if(last_usage)
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

		for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
	}
}
