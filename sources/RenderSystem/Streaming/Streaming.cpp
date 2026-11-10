module Graphics:Streaming;

import :Streaming;

import Core;
import HAL;

namespace Streaming
{
	Streamer::Streamer(Backend& backend, uint32_t max_page_ids, Policy policy)
		: backend(backend)
		, policy(policy)
	{
		page_refs.resize(max_page_ids);
	}

	Streamer::~Streamer()
	{
		stop();
	}

	void Streamer::stop()
	{
		{
			std::lock_guard<std::mutex> g(queue_mutex);
			stopping = true;
		}
		wake.notify_all();
		if (thread.joinable())
			thread.join();
	}

	void Streamer::start()
	{
		if (!thread.joinable())
			thread = std::thread([this] { run(); });
	}

	void Streamer::submit(Requests&& requests)
	{
		{
			std::lock_guard<std::mutex> g(queue_mutex);
			queue.push_back(std::move(requests));
			if (queue.size() > 2)
				queue.pop_front();
		}
		wake.notify_one();
	}

	// `count` more slots with memory: the released ones come back first, then new numbers.
	void Streamer::grow(uint32_t count)
	{
		while (count && !released_slots.empty())
		{
			const uint32_t slot = released_slots.back();
			released_slots.pop_back();
			backend.restore_slot(slot);
			free_slots.push_back(slot);
			count--;
		}

		if (count)
		{
			const uint32_t first = static_cast<uint32_t>(pool_size);
			backend.allocate_pool(count);
			pool_size += count;

			// The lowest new slot is taken first.
			for (uint32_t slot = static_cast<uint32_t>(pool_size); slot-- > first;)
				free_slots.push_back(slot);
		}

		// The new slots' memory is mapped by the next prepare(); until that list is done DirectStorage must not write them.
		pool_version++;
		pool_map_pending = true;
		pool_mapped_flag = false;
	}

	void Streamer::ensure_pool(uint32_t count)
	{
		const uint32_t live = static_cast<uint32_t>(pool_size - released_slots.size());
		if (count > live)
			grow(count - live);
	}

	void Streamer::ensure_free(uint32_t count)
	{
		if (free_slots.size() < count)
			grow(static_cast<uint32_t>(count - free_slots.size()));
	}

	// Free slots kept for the next loads, so a steady trickle of them does not make and free memory every frame.
	static constexpr size_t SlotChunk = 16;
	static constexpr size_t SlotSlack = 4 * SlotChunk;

	void Streamer::trim_pool()
	{
		if (!on_demand)
			return;

		// What the budget can still take, up to the slack, is kept free; the rest of the free slots only hold memory.
		const size_t room = std::min<size_t>(streamed_in_use < budget ? budget - streamed_in_use : 0, SlotSlack);
		if (free_slots.size() <= room)
			return;

		// Highest numbers go: the pool's memory was taken in numbered order, so whole heap pages empty out.
		std::sort(free_slots.begin(), free_slots.end(), std::greater<uint32_t>());
		const size_t surplus = free_slots.size() - room;
		for (size_t i = 0; i < surplus; i++)
		{
			backend.release_slot(free_slots[i]);
			released_slots.push_back(free_slots[i]);
		}
		free_slots.erase(free_slots.begin(), free_slots.begin() + surplus);
	}


	void Streamer::add(Resource& resource)
	{
		const uint32_t base = static_cast<uint32_t>(resource.page_id_base());
		resources.push_back(&resource);
		for (uint32_t p = 0; p < resource.states.size(); p++)
			page_refs[base + p] = { &resource, p };
	}

	void Streamer::remove(Resource& resource)
	{
		changed.erase(&resource);

		auto it = std::find(resources.begin(), resources.end(), &resource);
		if (it == resources.end())
			return;
		resources.erase(it);

		const uint32_t base = static_cast<uint32_t>(resource.page_id_base());
		for (uint32_t p = 0; p < resource.states.size(); p++)
		{
			PageState& state = resource.states[p];
			if (state.slot != InvalidSlot)
			{
				retiring.push_back(state.slot);
				if (resource.streamed)
					streamed_in_use--;
			}
			page_refs[base + p] = {};
		}

		loading.erase(std::remove_if(loading.begin(), loading.end(), [&](const Loading& l) { return l.resource == &resource; }), loading.end());
	}

	uint32_t Streamer::take_slot()
	{
		const uint32_t slot = free_slots.back();
		free_slots.pop_back();
		return slot;
	}

	uint32_t Streamer::load_root(Resource& resource, uint32_t page)
	{
		ASSERT(!free_slots.empty());
		PageState& state = resource.states[page];
		state.slot = take_slot();
		state.status = PageState::Status::Resident;
		state.last_touched = current_epoch;
		if (resource.streamed)
			streamed_in_use++;
		backend.begin_load(resource, page, state.slot, false);   // shown at once, so it cannot wait for a read
		sync_flags(resource, page);
		return state.slot;
	}

	void Streamer::sync_flags(Resource& resource, uint32_t page)
	{
		const PageState& state = resource.states[page];
		uint32_t flags = 0;
		if (state.status == PageState::Status::Loading)
			flags |= FlagLoading;
		if (state.status == PageState::Status::Resident && !resource.is_root(page) && state.resident_children == 0)
			flags |= FlagEvictable;
		backend.set_flags(resource, page, flags, state.last_touched);
	}

	Stats Streamer::stats()
	{
		std::lock_guard<std::mutex> g(m);
		Stats out;
		out.pool_pages = static_cast<uint32_t>(pool_size - released_slots.size());
		out.free_pages = static_cast<uint32_t>(free_slots.size());
		out.loading_pages = static_cast<uint32_t>(loading.size());
		out.loads = loads;
		out.file_loads = file_loads;
		out.evictions = evictions;
		out.snapshots = requests_done;
		out.wanted = last_wanted;
		out.dropped = dropped_requests;
		return out;
	}

	void Streamer::prepare(HAL::CommandList::ptr& list)
	{
		// The page-table writes of the changes made since the last prepare are in this list: draws recorded after it see the new cut.
		for (Resource* resource : changed)
			resource->generation.fetch_add(1, std::memory_order_relaxed);
		changed.clear();

		std::weak_ptr<Streamer> weak = weak_from_this();

		if (pool_map_pending)
		{
			// This list carries the pool's mapping; once it is done DirectStorage may write the slots, unless the pool
			// has grown since: that growth's mapping is in a later list.
			pool_map_pending = false;
			list->on_done([weak, version = pool_version]
			{
				auto self = weak.lock();
				if (!self)
					return;
				std::lock_guard<std::mutex> g(self->m);
				if (version == self->pool_version)
					self->pool_mapped_flag = true;
			});
		}

		if (!retiring.empty())
		{
			// The table writes above hide these slots from every frame recorded after this list; the frames
			// already submitted may still be reading them, and they all finish before this list does.
			retiring_inflight += retiring.size();
			list->on_done([weak, slots = std::move(retiring)]
			{
				auto self = weak.lock();
				if (!self)
					return;
				std::lock_guard<std::mutex> g(self->m);
				self->free_slots.insert(self->free_slots.end(), slots.begin(), slots.end());
				self->retiring_inflight -= slots.size();
				self->trim_pool();
			});
			retiring.clear();
		}
	}

	void Streamer::run()
	{
		for (;;)
		{
			Requests requests;
			bool have_requests = false;
			{
				// Wakes without requests too, to show pages whose DirectStorage reads have finished.
				std::unique_lock<std::mutex> lock(queue_mutex);
				wake.wait_for(lock, std::chrono::milliseconds(2), [&] { return stopping || !queue.empty(); });
				if (stopping)
					return;
				if (!queue.empty())
				{
					requests = std::move(queue.front());
					queue.pop_front();
					have_requests = true;
				}
			}

			if (have_requests)
			{
				process(requests);
			}
			else
			{
				std::lock_guard<std::mutex> g(m);
				if (!loading.empty())
					publish();
			}
		}
	}

	// One set of requests: start loading what the draw wanted and lacked, making room from the idle pages the GPU found.
	void Streamer::process(const Requests& requests)
	{
		PROFILE(L"streamer");
		std::lock_guard<std::mutex> g(m);

		victims = requests.victims;
		dropped_requests += requests.dropped;

		std::vector<Want> wanted;
		for (const Want& want : requests.wanted)
		{
			if (want.id >= page_refs.size() || !page_refs[want.id].resource)
				continue;

			const PageRef& ref = page_refs[want.id];
			if (ref.resource->states[ref.page].status == PageState::Status::Missing)
				wanted.push_back(want);
		}
		last_wanted = static_cast<uint32_t>(wanted.size());
		requests_done++;

		std::sort(wanted.begin(), wanted.end(), [](const Want& a, const Want& b) { return a.priority > b.priority; });

		// Each wanted page with its missing ancestors ahead of it.
		std::vector<PageRef> order;
		std::unordered_set<uint32_t> queued;
		auto add = [&](auto&& self, Resource& resource, uint32_t page) -> void
		{
			if (resource.states[page].status != PageState::Status::Missing)
				return;
			if (!queued.insert(static_cast<uint32_t>(resource.page_id_base() + page)).second)
				return;
			for (uint32_t parent : resource.states[page].parents)
				self(self, resource, parent);
			order.push_back({ &resource, page });
		};
		for (const Want& w : wanted)
		{
			if (order.size() >= policy.max_loads_per_snapshot)
				break;
			add(add, *page_refs[w.id].resource, page_refs[w.id].page);
		}

		// What streamed pages may still take. Slots already on their way back (hidden, waiting for their frames to
		// finish) are no longer counted in it.
		if (streamed_in_use > budget)
			evict(streamed_in_use - budget, requests.epoch, true);

		const size_t available = streamed_in_use < budget ? budget - streamed_in_use : 0;
		if (order.size() > available)
			evict(order.size() - available, requests.epoch);

		if (on_demand)
		{
			// Memory for the loads about to start, a chunk at a time, never past what the budget allows.
			const size_t room = streamed_in_use < budget ? budget - streamed_in_use : 0;
			const size_t need = std::min(order.size(), room);
			if (free_slots.size() < need)
			{
				const size_t chunks = (need - free_slots.size() + SlotChunk - 1) / SlotChunk;
				grow(static_cast<uint32_t>(std::min(chunks * SlotChunk, room - free_slots.size())));
			}
		}

		for (const PageRef& ref : order)
		{
			if (free_slots.empty() || streamed_in_use >= budget)
				break;

			PageState& state = ref.resource->states[ref.page];
			state.slot = take_slot();
			streamed_in_use++;
			state.status = PageState::Status::Loading;
			Backend::Load load = backend.begin_load(*ref.resource, ref.page, state.slot, true);
			if (load.from_file)
				file_loads++;
			sync_flags(*ref.resource, ref.page);
			loading.push_back({ ref.resource, ref.page, std::move(load) });
			loads++;
		}

		publish();
	}

	// Shows every loaded page whose parents are shown, parents first.
	void Streamer::publish()
	{
		bool progress = true;
		while (progress)
		{
			progress = false;
			for (size_t i = 0; i < loading.size();)
			{
				Loading item = loading[i];
				PageState& state = item.resource->states[item.page];

				if (item.load.from_file && !(item.load.poll ? item.load.poll() : item.load.waiter.is_completed()))
				{
					i++;
					continue;
				}

				bool parents_shown = true;
				for (uint32_t parent : state.parents)
					parents_shown = parents_shown && item.resource->states[parent].status == PageState::Status::Resident;

				if (!parents_shown)
				{
					i++;
					continue;
				}

				backend.show(*item.resource, item.page, state.slot);
				state.status = PageState::Status::Resident;
				state.last_touched = current_epoch;
				changed.insert(item.resource);
				sync_flags(*item.resource, item.page);
				for (uint32_t parent : state.parents)
				{
					item.resource->states[parent].resident_children++;
					sync_flags(*item.resource, parent);
				}

				loading.erase(loading.begin() + i);
				progress = true;
			}
		}
	}

	// Hides up to `count` of the GPU's idle pages, least recently reached first, to make room. Their slots return to
	// the pool once the frames that may still read them are done (prepare).
	void Streamer::evict(size_t count, uint32_t epoch, bool any)
	{
		struct Candidate
		{
			uint32_t last_touched;
			PageRef ref;
		};
		std::vector<Candidate> candidates;

		if (any)
		{
			for (Resource* resource : resources)
				for (uint32_t page = 0; page < resource->states.size(); page++)
				{
					const PageState& state = resource->states[page];
					if (state.status == PageState::Status::Resident && !resource->is_root(page) && state.resident_children == 0)
						candidates.push_back({ state.last_touched, { resource, page } });
				}
		}

		// The GPU decided from flags that may be a frame or two old, so each one is checked again.
		for (const Victim& victim : victims)
		{
			if (any || victim.id >= page_refs.size() || !page_refs[victim.id].resource)
				continue;

			const PageRef& ref = page_refs[victim.id];
			const PageState& state = ref.resource->states[ref.page];
			if (state.status == PageState::Status::Resident && !ref.resource->is_root(ref.page)
				&& state.resident_children == 0 && victim.last_touched + policy.eviction_age < epoch)
				candidates.push_back({ victim.last_touched, ref });
		}

		std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.last_touched < b.last_touched; });

		for (size_t i = 0; i < std::min(count, candidates.size()); i++)
		{
			Resource& resource = *candidates[i].ref.resource;
			const uint32_t page = candidates[i].ref.page;
			PageState& state = resource.states[page];

			backend.hide(resource, page);
			changed.insert(&resource);

			retiring.push_back(state.slot);
			streamed_in_use--;
			state.slot = InvalidSlot;
			state.status = PageState::Status::Missing;
			sync_flags(resource, page);
			for (uint32_t parent : state.parents)
			{
				resource.states[parent].resident_children--;
				sync_flags(resource, parent);
			}
			evictions++;
		}

		// What was used is gone from the pool; the next request set brings fresh candidates.
		if (!any)
			victims.clear();
	}
}
