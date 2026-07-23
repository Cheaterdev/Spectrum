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

	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		ASSERT(resource);

		ASSERT(IsFullySupport(type, before));
		ASSERT(IsFullySupport(type, after));

		if (check(after.access & (BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ)))
			ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));

		barriers.emplace_back(Barrier{ const_cast<Resource*>(resource), before, after, subres, flags });
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
		skip_end_decay = false;
		first_usage = nullptr;
		last_usage = nullptr;
	}

	void ResourceListStateCPU::check_valid(const Resource* resource)
	{
#ifdef DEV
		if (resource && check(resource->get_desc().Flags & ResFlags::DisableStateTracking)) return;

		auto usage = last_usage;
		auto prev_usage = last_usage->prev_usage;

		if(prev_usage && prev_usage->prev_usage && prev_usage->wanted_state!=ResourceStates::UNKNOWN)
			ASSERT( prev_usage->wanted_state.operation!=BarrierSync::NONE);
#endif
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

	ResourceListStateCPU& SubResourcesCPU::slot(UINT id)
	{
		return (uniform || id == ALL_SUBRESOURCES) ? uniform_state : subres[id];
	}

	const ResourceListStateCPU& SubResourcesCPU::slot(UINT id) const
	{
		return (uniform || id == ALL_SUBRESOURCES) ? uniform_state : subres[id];
	}

	void SubResourcesCPU::expand(UINT count)
	{
		if (!uniform) return;

		// Materialize per-subres slots from the shared uniform chain: each slot
		// copies uniform_state's first/last_usage (sharing the history nodes,
		// which carry subres == ALL_SUBRESOURCES). The next per-subres
		// transition forks its own node after that shared history — correct, at
		// the cost of one extra barrier for that subresource on the frame it
		// first diverges (design A).
		if (subres.size() < count) subres.resize(count);
		for (UINT i = 0; i < count; i++) subres[i] = uniform_state;
		uniform = false;
	}

	void SubResourcesCPU::reset()
	{
		used = false;
		uniform = true;
		batch_touch_id = 0;   // stale batch id from a prior frame must not match
		uniform_state.reset();
		for (auto& s : subres)
			s.reset();
	}

	CommandListType SubResourcesCPU::get_best_list_type_last()
	{
		CommandListType type = CommandListType::COPY;

		auto one = [&](const ResourceListStateCPU& cpu) {
			if (!cpu.used) return;
			type = Merge(type, cpu.last_usage->wanted_state.get_best_cmd_type());
		};

		if (uniform) one(uniform_state);
		else for (auto& cpu : subres) one(cpu);

		return type;
	}

	CommandListType SubResourcesCPU::get_best_list_type_first()
	{
		CommandListType type = CommandListType::COPY;

		auto one = [&](const ResourceListStateCPU& cpu) {
			if (!cpu.used) return;
			type = Merge(type, cpu.first_usage->wanted_state.get_best_cmd_type());
		};

		if (uniform) one(uniform_state);
		else for (auto& cpu : subres) one(cpu);

		return type;
	}

	const ResourceListStateCPU& SubResourcesCPU::get_subres_state(UINT id) const
	{
		return slot(id);
	}

	ResourceListStateCPU& SubResourcesCPU::get_subres_state(UINT id)
	{
		return slot(id);
	}

	ResourceUsage* SubResourcesCPU::get_first_usage(UINT id) const
	{
		return slot(id).first_usage;
	}

	ResourceUsage* SubResourcesCPU::get_last_usage(UINT id) const
	{
		return slot(id).last_usage;
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
			//ASSERT(my_state.first_usage == my_state.last_usage);
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
		// ALL_SUBRESOURCES resolves to subresource 0 as the representative — used
		// by the uniform CPU fast path, where the GPU layout is uniform anyway.
		return subres[id == ALL_SUBRESOURCES ? 0 : id];
	}

	ResourceListStateGPU& SubResourcesGPU::get_subres_state(UINT id)
	{
		return subres[id == ALL_SUBRESOURCES ? 0 : id];
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

	ResourceState ResourceStateManager::get_desired_state() const
	{
		auto flags = resource->get_desc().Flags;
		if (check(flags & ResFlags::ShaderResource))  return ResourceStates::SHADER_RESOURCE;
		if (check(flags & ResFlags::UnorderedAccess)) return ResourceStates::UNORDERED_ACCESS;
		// No read usage declared — leave it in the copy-destination state.
		return ResourceStates::COPY_DEST;
	}

	void ResourceStateManager::set_resting_state(TextureLayout layout)
	{
		initial_layout = layout;
		for (auto& e : gpu_state.subres)
			e.layout = layout;
		virgin = false;
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
				state.uniform = true;             // start each list in the uniform fast path
				state.batch_touch_id = 0;
				state.uniform_state.reset();
				state.subres.resize(count);
				for (auto& e : state.subres)
				{
					e.used = false;   // prevents set_cpu_state_first from using stale layout
					e.skip_end_decay = false;
				}
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

		// Captured before the per-subres loop so every subresource of this
		// transition sees the same virginity (the flag flips once, below).
		const bool was_virgin = virgin;

		// `subres` here is the value stamped onto the emitted usage node: a real
		// subresource index in expanded mode, or ALL_SUBRESOURCES in uniform mode
		// (one node → one all-subresources barrier).
		auto transition_one = [&](UINT subres) {

			auto& subres_cpu = cpu_state.slot(subres);
			if (!subres_cpu.used)
			{
				if (!check(resource->get_desc().Flags & ResFlags::DisableStateTracking) && resource->get_desc().is_texture())
				{
					// Seed node = the state the resource is IN at this list's first
					// touch. Non-FG virgin resource whose first-ever use is a WRITE
					// (incl. COPY_DEST upload) → UNKNOWN so compile_transitions
					// emits a DISCARD activation (R4): contents are meaningless and
					// about to be overwritten. A read-first virgin resource is
					// initialized out-of-band, so must not be discarded.
					//
					// Otherwise seed from the TRACKED layout (gpu_state), not the
					// creation-time initial_layout. initial_layout is only a guess
					// and is wrong for any resource whose layout has since moved —
					// e.g. an adopted history-prev, which carries the layout last
					// frame's `current` ended in. Seeding the guess emitted barriers
					// whose LayoutBefore disagreed with the real layout
					// (D3D12 #1334 INCOMPATIBLE_BARRIER_LAYOUT). gpu_state equals
					// initial_layout for a freshly created resource, so this is a
					// strict improvement.
					auto target = ResourceStates::NO_ACCESS;
					if (!resource->frame_graph_managed && was_virgin && state.has_write_bits())
						target = ResourceStates::UNKNOWN;
					else
						target.layout = gpu_state.get_subres_state(subres).layout;

					subres_cpu.used = true;
					subres_cpu.first_usage = subres_cpu.last_usage = (list)->add_usage((resource), subres, target);

					//	ASSERT(!(!check(subres_cpu.first_usage->resource->get_desc().Flags & ResFlags::DisableStateTracking) && subres_cpu.first_usage->wanted_state.access == ResourceStates::NO_ACCESS.access));

					auto transition = (list)->add_usage((resource), subres, state);
					subres_cpu.add_usage(transition);
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
		};


		if (s != ALL_SUBRESOURCES)
		{
			// Subresource-scoped touch: leave the uniform mode (if any) and track
			// this subresource independently from here on.
			if (cpu_state.uniform)
				cpu_state.expand((UINT)gpu_state.subres.size());
			transition_one(s);
		}
		else if (cpu_state.uniform)
		{
			// Whole-resource touch while still uniform: one shared node.
			transition_one(ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++)
				transition_one(i);
		}

		// First use has now been recorded — later uses preserve contents
		// (no more DISCARD). Re-discarding would only ever be safe anyway, but
		// non-FG resources persist data across frames, so this must flip once.
		virgin = false;
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

		// Uniform CPU state → one pass stamping ALL_SUBRESOURCES; else per-subres.
		if (cpu_state.uniform)
		{
			merge_one(ALL_SUBRESOURCES);
			transition_one(ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) merge_one(i);
			for (int i = 0; i < gpu_state.subres.size(); i++) transition_one(i);
		}

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

		// Uniform CPU state → one pass stamping ALL_SUBRESOURCES; else per-subres.
		if (cpu_state.uniform)
		{
			merge_one(ALL_SUBRESOURCES);
			transition_one(ALL_SUBRESOURCES);
		}
		else
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) merge_one(i);
			for (int i = 0; i < gpu_state.subres.size(); i++) transition_one(i);
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

		if (cpu_state.uniform)
		{
			// Uniform CPU state ⟺ uniform GPU handoff: subresource 0 represents
			// all. One ALL_SUBRESOURCES transition keeps the resource uniform.
			if (gpu_state.subres[0].layout != TextureLayout::NONE)
			{
				auto target = ResourceStates::NO_ACCESS;
				target.layout = gpu_state.subres[0].layout;
				transition(from, target, ALL_SUBRESOURCES);
			}
		}
		else
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
			if(last_usage)
			last_usage->last_point = (list)->get_last_usage_point();
		};

		if (state.used)
		{
			for (int i = 0; i < gpu_state.subres.size(); i++) transit(i);
		}
	}

	void ResourceStateManager::chain_lists(Transitions* from, Transitions* to) const
	{
		if (resource->get_desc().is_buffer()) return;

		auto& prev = get_state(from);
		auto& next = get_state(to);

		if (!prev.used || !next.used) return;

		// A SYNC_NONE / NO_ACCESS node: the first-touch seed planted by
		// transition_one, a zero-transition prepended by prepare_state, or a
		// release appended by prepare_after_state. A list can carry several of
		// them at either end, around its real usages.
		auto is_none = [](const ResourceUsage* u)
		{
			return u->wanted_state.operation == BarrierSync::NONE
				&& u->wanted_state.access == BarrierAccess::NO_ACCESS;
		};

		auto one = [&](UINT subres)
		{
			auto* last  = prev.slot(subres).last_usage;
			auto* first = next.slot(subres).first_usage;

			if (!last || !first) return;

			// Walk forward over `to`'s SYNC_NONE prefix to its first REAL usage.
			auto* target = first;
			while (is_none(target) && target->next_usage)
				target = target->next_usage;

			// `to` has NO real usage — only SYNC_NONE reconciliation nodes
			// (redundant seed + release from prepare_state/prepare_after_state). If
			// `from` really accessed the resource, `to`'s first node re-declares the
			// state with SyncBefore=NONE AFTER that access → #1417. Walk `from` back
			// to its last REAL usage and hand it to `to`'s first node so the barrier
			// carries a real SyncBefore; suppress `from`'s own end release (the
			// resource stays live for `to`). The remaining `to` nodes now chain off a
			// real predecessor instead of a SYNC_NONE seed.
			if (is_none(target))
			{
				while (is_none(last) && last->prev_usage)
					last = last->prev_usage;

				if (!is_none(last))
				{
					// `to`'s LAST node is its exit release; the earlier none nodes are
					// redundant re-seeds. Suppress the seeds (so none emit a bare
					// SyncBefore=NONE after the access) and hand the release `from`'s
					// last real usage as predecessor so it carries real sync.
					auto* lastnode = first;
					while (lastnode->next_usage) lastnode = lastnode->next_usage;

					for (auto* u = first; u != lastnode; u = u->next_usage)
						u->suppressed = true;

					if (!lastnode->prev_usage || is_none(lastnode->prev_usage))
						lastnode->prev_usage = last;

					// Suppress `from`'s OWN trailing release nodes for this subresource
					// too — the resource stays live for `to` (and whatever real consumer
					// follows it). Setting skip_end_decay alone only stops the generic
					// transition_one end-decay; the explicit prepare_after_state release
					// nodes appended between `last` and `from`'s recorded last_usage still
					// publish DS_WRITE->NO_ACCESS (SyncAfter=NONE) unless suppressed here,
					// which is exactly the #1417 seen on PSSM_Depths' non-rendered array
					// slices (each cascade needs the whole array but renders one slice, so
					// the other slices resolve to this reconciliation-only branch).
					for (auto* u = prev.slot(subres).last_usage; u != last; u = u->prev_usage)
						u->suppressed = true;

					prev.slot(subres).skip_end_decay = true;
				}
				return;
			}

			// Walk back over `from`'s SYNC_NONE suffix to its last REAL usage.
			// Chaining onto a release node would emit `SyncBefore == NONE`, which
			// is precisely the thing being avoided.
			while (is_none(last) && last->prev_usage)
				last = last->prev_usage;

			if (is_none(last)) return;

			// Keep `from`'s subresource live past the boundary — suppress its
			// trailing SYNC_NONE suffix and stop the generic end-of-list release.
			// This runs UNCONDITIONALLY (before the predecessor guard below): when
			// `to` expanded from a uniform `need`, several of its subresources
			// resolve to ONE shared usage node. The first sibling chains that node
			// and sets its prev_usage; without suppressing here first, every later
			// sibling would hit the "predecessor already real" guard and return
			// with `from`'s decay for THAT subresource still in place — the release
			// publishes SyncAfter == NONE and `to` then touches the subresource in
			// the same ECL scope (#1417). The decay must be dropped per subresource
			// `to` actually uses; the shared node only needs one predecessor.
			for (auto* u = prev.slot(subres).last_usage; u != last; u = u->prev_usage)
				u->suppressed = true;

			prev.slot(subres).skip_end_decay = true;

			// Replace only a missing or SYNC_NONE predecessor; never clobber a
			// real one (already chained by a sibling subresource sharing this node,
			// or a genuine in-list transition). A leading SYNC_NONE prefix on `to`
			// would publish SyncBefore == NONE after the resource was touched, so
			// suppress it too — but only on the path that actually (re)chains.
			if (target->prev_usage && !is_none(target->prev_usage)) return;

			for (auto* u = first; u != target; u = u->next_usage)
				u->suppressed = true;

			target->prev_usage = last;
		};

		if (next.uniform)
		{
			if (prev.uniform)
			{
				// Both sides track the whole resource as one chain.
				one(ALL_SUBRESOURCES);
			}
			else
			{
				// `next` reads the whole resource through a single shared node, but
				// `prev` left it in per-subresource tracking. The one() loop cannot be
				// used here: its first iteration chains the shared node, and every later
				// iteration then hits the "predecessor already real" guard and returns
				// BEFORE suppressing `prev`'s release for that subresource. Those
				// releases publish SyncAfter == NONE, and the shared read node still
				// touches subres 1..N in the same ECL scope (#1417), while their stale
				// per-subres before-layout drives #1334. Suppress `prev`'s exit release
				// for EVERY subresource it used, and hand the shared node one real
				// predecessor (the resting read state is uniform across subresources, so
				// any real last-usage is a valid before-state for the ALL barrier).
				auto* first = next.uniform_state.first_usage;
				if (first)
				{
					auto* target = first;
					while (is_none(target) && target->next_usage)
						target = target->next_usage;

					ResourceUsage* rep_last = nullptr;

					// Per-subresource before-state as seen at this boundary, defaulted
					// to the read state so untouched / already-matching subresources
					// become no-ops in the split path below.
					std::vector<ResourceState> subres_before(gpu_state.subres.size(), target->wanted_state);

					for (UINT i = 0; i < gpu_state.subres.size(); i++)
					{
						auto* last = prev.slot(i).last_usage;
						if (!last) continue;

						while (is_none(last) && last->prev_usage)
							last = last->prev_usage;
						if (is_none(last)) continue;   // `prev` never really touched subres i

						// Keep subres i live past the boundary: drop its trailing
						// SYNC_NONE release and stop the generic end-of-list decay.
						for (auto* u = prev.slot(i).last_usage; u != last; u = u->prev_usage)
							u->suppressed = true;
						prev.slot(i).skip_end_decay = true;

						subres_before[i] = last->wanted_state;   // this subresource's real before-state
						rep_last = last;
					}

					if (rep_last)
					{
						if (is_none(target))
						{
							// `next` carries only SYNC_NONE nodes — hand its exit release
							// a real predecessor and suppress the redundant seeds.
							auto* lastnode = first;
							while (lastnode->next_usage) lastnode = lastnode->next_usage;
							for (auto* u = first; u != lastnode; u = u->next_usage)
								u->suppressed = true;
							if (!lastnode->prev_usage || is_none(lastnode->prev_usage))
								lastnode->prev_usage = rep_last;
						}
						else
						{
							for (auto* u = first; u != target; u = u->next_usage)
								u->suppressed = true;
							if (!target->prev_usage || is_none(target->prev_usage))
								target->prev_usage = rep_last;

							// If `prev` left the subresources in DIFFERENT states, a single
							// ALL barrier cannot express it (it would carry one before-layout
							// and mismatch the rest -> #1334/#1417). Record the per-subres
							// before-states so compile_transitions emits a split barrier;
							// after it the resource is uniform in the read state again. When
							// the states ARE uniform, keep the single ALL barrier (cheaper).
							bool split = false;
							for (UINT i = 1; i < subres_before.size(); i++)
								if (!(subres_before[i] == subres_before[0])) { split = true; break; }

							if (split)
								target->split_before = std::move(subres_before);
						}
					}
				}
			}
		}
		else
		{
			for (UINT i = 0; i < gpu_state.subres.size(); i++)
				one(i);
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
