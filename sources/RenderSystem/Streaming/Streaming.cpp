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

	void Streamer::ensure_pool(uint32_t count)
	{
		if (pool_size != 0)
		{
			if (pool_size == count || !streamed_list.empty())
				return;

			// Nothing streams from the pool any more, so it can take the size asked for since.
			backend.free_pool();
			pool_size = 0;
			free_slots.clear();
			retiring.clear();
			retiring_inflight = 0;
			pool_generation++;
			pool_mapped_flag = false;
		}

		pool_map_pending = true;
		pool_size = count;
		backend.allocate_pool(count);
		for (uint32_t i = 0; i < count; i++)
			free_slots.push_back(count - 1 - i);
	}

	void Streamer::add(Resource& resource)
	{
		const uint32_t base = static_cast<uint32_t>(resource.page_id_base());
		streamed_list.push_back(&resource);
		for (uint32_t p = 0; p < resource.states.size(); p++)
			page_refs[base + p] = { &resource, p };
	}

	void Streamer::remove(Resource& resource)
	{
		changed.erase(&resource);
		if (!resource.streamed)
			return;

		const uint32_t base = static_cast<uint32_t>(resource.page_id_base());
		for (uint32_t p = 0; p < resource.states.size(); p++)
		{
			PageState& state = resource.states[p];
			if (state.slot != InvalidSlot)
				retiring.push_back(state.slot);
			page_refs[base + p] = {};
		}

		loading.erase(std::remove_if(loading.begin(), loading.end(), [&](const Loading& l) { return l.resource == &resource; }), loading.end());
		streamed_list.erase(std::remove(streamed_list.begin(), streamed_list.end(), &resource), streamed_list.end());
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
		backend.begin_load(resource, page, state.slot, false);   // shown at once, so it cannot wait for a read
		return state.slot;
	}

	Stats Streamer::stats()
	{
		std::lock_guard<std::mutex> g(m);
		Stats out;
		out.pool_pages = static_cast<uint32_t>(pool_size);
		out.free_pages = static_cast<uint32_t>(free_slots.size());
		out.loading_pages = static_cast<uint32_t>(loading.size());
		out.loads = loads;
		out.file_loads = file_loads;
		out.evictions = evictions;
		out.snapshots = requests_done;
		out.wanted = last_wanted;
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
			// This list carries the pool's mapping; once it is done DirectStorage may write the slots.
			pool_map_pending = false;
			list->on_done([weak, generation = pool_generation]
			{
				auto self = weak.lock();
				if (!self)
					return;
				std::lock_guard<std::mutex> g(self->m);
				if (generation == self->pool_generation)
					self->pool_mapped_flag = true;
			});
		}

		if (!retiring.empty())
		{
			// The table writes above hide these slots from every frame recorded after this list; the frames
			// already submitted may still be reading them, and they all finish before this list does.
			retiring_inflight += retiring.size();
			list->on_done([weak, slots = std::move(retiring), generation = pool_generation]
			{
				auto self = weak.lock();
				if (!self)
					return;
				std::lock_guard<std::mutex> g(self->m);
				if (generation != self->pool_generation)
					return;   // the pool they came from was rebuilt
				self->free_slots.insert(self->free_slots.end(), slots.begin(), slots.end());
				self->retiring_inflight -= slots.size();
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

	// One set of requests: refresh the LRU stamps, then start loading what the draw wanted and lacked.
	void Streamer::process(const Requests& requests)
	{
		PROFILE(L"streamer");
		std::lock_guard<std::mutex> g(m);

		for (const Touch& touch : requests.touched)
		{
			if (touch.id >= page_refs.size() || !page_refs[touch.id].resource)
				continue;

			const PageRef& ref = page_refs[touch.id];
			PageState& state = ref.resource->states[ref.page];
			if (state.status == PageState::Status::Resident && touch.epoch > state.last_touched)
				state.last_touched = touch.epoch;
		}

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

		// Slots already on their way back (hidden, waiting for their frames to finish) count as available.
		const size_t available = free_slots.size() + retiring.size() + retiring_inflight;
		if (order.size() > available)
			evict(order.size() - available, requests.epoch);

		for (const PageRef& ref : order)
		{
			if (free_slots.empty())
				break;

			PageState& state = ref.resource->states[ref.page];
			state.slot = take_slot();
			state.status = PageState::Status::Loading;
			Backend::Load load = backend.begin_load(*ref.resource, ref.page, state.slot, true);
			if (load.from_file)
				file_loads++;
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

				if (item.load.from_file && !item.load.waiter.is_completed())
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
				for (uint32_t parent : state.parents)
					item.resource->states[parent].resident_children++;

				loading.erase(loading.begin() + i);
				progress = true;
			}
		}
	}

	// Hides up to `count` pages, least recently reached first, to make room. Their slots return to the pool once
	// the frames that may still read them are done (prepare).
	void Streamer::evict(size_t count, uint32_t epoch)
	{
		struct Victim
		{
			uint32_t last_touched;
			PageRef ref;
		};
		std::vector<Victim> victims;

		for (Resource* resource : streamed_list)
			for (uint32_t p = 0; p < resource->states.size(); p++)
			{
				const PageState& state = resource->states[p];
				if (state.status == PageState::Status::Resident && !resource->is_root(p)
					&& state.resident_children == 0 && state.last_touched + policy.eviction_age < epoch)
					victims.push_back({ state.last_touched, { resource, p } });
			}

		std::sort(victims.begin(), victims.end(), [](const Victim& a, const Victim& b) { return a.last_touched < b.last_touched; });

		for (size_t i = 0; i < std::min(count, victims.size()); i++)
		{
			Resource& resource = *victims[i].ref.resource;
			PageState& state = resource.states[victims[i].ref.page];

			backend.hide(resource, victims[i].ref.page);
			changed.insert(&resource);
			for (uint32_t parent : state.parents)
				resource.states[parent].resident_children--;

			retiring.push_back(state.slot);
			state.slot = InvalidSlot;
			state.status = PageState::Status::Missing;
			evictions++;
		}
	}
}
