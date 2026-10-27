module Graphics:QuantaResidency;

import :QuantaResidency;

import Core;
import HAL;
import :Quanta;

import RenderSystem;

namespace
{
	VariableContext& streaming_context()
	{
		static auto ctx = VariableContext::create(L"Quanta streaming");
		return *ctx;
	}

	// New meshes keep only their root pages resident and load the rest as the draw asks for them.
	Variable<bool> g_streaming = { false, "Streaming", &streaming_context() };

	// The pages buffer slots the streamed pages share, committed when the first streamed mesh arrives.
	Variable<int> g_pool_pages = { 1024, "Pool pages", &streaming_context(), 2, 4096 };

	// Pages of a mesh loaded from an asset file are read straight into their pool slot by DirectStorage.
	Variable<bool> g_direct_storage = { true, "Direct storage", &streaming_context() };

	// Pages loaded per feedback snapshot at most: a burst of requests would otherwise stall the frame.
	constexpr size_t MaxLoadsPerSnapshot = 64;

	// A page the draw last reached this many frames ago is fair game for eviction.
	constexpr uint32_t EvictionAge = 3;
}

namespace Quanta
{
	bool streaming_enabled()
	{
		return g_streaming;
	}

	void set_streaming(bool enabled, uint32_t pool_pages)
	{
		g_streaming = enabled;
		if (pool_pages)
			g_pool_pages = static_cast<int>(pool_pages);
	}

	Residency::Residency()
		: pages(RenderSystem::get().device(), MaxPageDwords)
		, groups(RenderSystem::get().device(), MaxGroupDwords)
		, nodes(RenderSystem::get().device(), MaxNodeDwords)
		, page_slots(RenderSystem::get().device(), MaxPageIds)
		, feedback(RenderSystem::get().device(), MaxPageIds * 2, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)
	{
		page_refs.resize(MaxPageIds);
	}

	Residency::~Residency()
	{
		{
			std::lock_guard<std::mutex> g(snapshot_mutex);
			stopping = true;
		}
		wake.notify_all();
		if (streamer.joinable())
			streamer.join();
	}

	Resident::~Resident()
	{
		if (!Residency::is_good())
			return;

		auto& residency = Residency::get();
		residency.release(*this);
		if (!streamed)
			residency.pages.free(page_handle);
		residency.page_slots.free(page_id_handle);
		residency.groups.free(group_handle);
		residency.nodes.free(node_handle);
	}

	void Residency::release(Resident& resident)
	{
		std::lock_guard<std::mutex> g(m);
		changed.erase(&resident);
		if (!resident.streamed)
			return;

		const uint32_t base = static_cast<uint32_t>(resident.page_id_base());
		for (uint32_t p = 0; p < resident.states.size(); p++)
		{
			PageState& state = resident.states[p];
			if (state.slot != InvalidIndex)
				retiring.push_back(state.slot);
			page_refs[base + p] = {};
		}

		loading.erase(std::remove_if(loading.begin(), loading.end(), [&](const Loading& l) { return l.resident == &resident; }), loading.end());
		streamed_list.erase(std::remove(streamed_list.begin(), streamed_list.end(), &resident), streamed_list.end());
	}

	void Residency::ensure_pool()
	{
		const size_t count = static_cast<size_t>(std::max(2, int(g_pool_pages)));
		if (!pool.empty())
		{
			if (pool.size() == count || !streamed_list.empty())
				return;

			// Nothing streams from the pool any more, so it can take the size asked for since.
			for (auto& handle : pool)
				pages.free(handle);
			pool.clear();
			free_slots.clear();
			retiring.clear();
			retiring_inflight = 0;
			pool_generation++;
			pool_mapped = false;
		}

		pool_map_pending = true;
		pool.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			pages.allocate(pool[i], PageDwords);
			free_slots.push_back(static_cast<uint32_t>(count - 1 - i));
		}
	}

	std::shared_ptr<Resident> Residency::acquire(const std::shared_ptr<Mesh>& mesh)
	{
		std::lock_guard<std::mutex> g(m);

		if (auto it = residents.find(mesh.get()); it != residents.end())
			if (auto existing = it->second.lock())
				return existing;

		const bool stream = g_streaming;

		auto resident = std::make_shared<Resident>();
		resident->mesh = mesh;
		resident->streamed = stream;

		if (stream)
			ensure_pool();
		else
			pages.allocate(resident->page_handle, mesh->pages.size() * PageDwords);
		page_slots.allocate(resident->page_id_handle, mesh->pages.size());
		groups.allocate(resident->group_handle, mesh->groups.size() * GroupDwords);
		nodes.allocate(resident->node_handle, mesh->nodes.size() * NodeDwords);

		const uint64_t group_base = resident->group_handle.get_offset() / GroupDwords;
		ASSERT(resident->group_handle.get_offset() % GroupDwords == 0 && resident->node_handle.get_offset() % NodeDwords == 0);

		const uint64_t page_id_base = resident->page_id_handle.get_offset();
		ASSERT(page_id_base + mesh->pages.size() <= MaxPageIds);
		max_page_id = std::max<uint32_t>(max_page_id, static_cast<uint32_t>(page_id_base + mesh->pages.size()));

		std::vector<uint> slots(mesh->pages.size(), InvalidIndex);
		if (stream)
		{
			resident->states.resize(mesh->pages.size());
			streamed_list.push_back(resident.get());
			find_parents(*resident);
			for (size_t p = 0; p < mesh->pages.size(); p++)
				page_refs[page_id_base + p] = { resident.get(), static_cast<uint32_t>(p) };

			// Roots are always there: they are what a missing page falls back to.
			for (uint32_t p = 0; p < mesh->pages.size(); p++)
				if (mesh->pages[p].root)
				{
					ASSERT(!free_slots.empty());
					PageState& state = resident->states[p];
					state.slot = take_slot();
					state.status = PageState::Status::Resident;
					state.last_touched = current_epoch;
					upload_page(*resident, p, state.slot, false);   // shown at once, so it cannot wait for a read
					slots[p] = static_cast<uint32_t>(pool[state.slot].get_offset());
				}
		}
		else
		{
			const uint64_t page_base = resident->page_handle.get_offset();
			for (size_t p = 0; p < mesh->pages.size(); p++)
			{
				const auto& data = mesh->pages[p].data;
				std::vector<uint> dwords((data.size() + 3) / 4, 0);
				std::memcpy(dwords.data(), data.data(), data.size());
				pages.write(page_base + p * PageDwords, dwords);
				slots[p] = static_cast<uint32_t>(page_base + p * PageDwords);
			}
		}
		page_slots.write(page_id_base, slots);

		std::vector<uint> group_dwords(mesh->groups.size() * GroupDwords);
		for (size_t i = 0; i < mesh->groups.size(); i++)
		{
			Group record = mesh->groups[i];
			record.page = static_cast<uint32_t>(page_id_base + record.page);
			record.family = pack_family(static_cast<uint32_t>(group_base + family_first(record.family)), family_count(record.family));
			std::memcpy(&group_dwords[i * GroupDwords], &record, sizeof(Group));
		}
		groups.write(resident->group_handle.get_offset(), group_dwords);

		std::vector<uint> node_dwords(mesh->nodes.size() * NodeDwords);
		for (size_t i = 0; i < mesh->nodes.size(); i++)
		{
			Node record = mesh->nodes[i];
			record.first_group = static_cast<uint32_t>(group_base + record.first_group);
			std::memcpy(&node_dwords[i * NodeDwords], &record, sizeof(Node));
		}
		nodes.write(resident->node_handle.get_offset(), node_dwords);

		if (stream && !streamer.joinable())
			streamer = std::thread([this] { run_streamer(); });

		residents[mesh.get()] = resident;
		return resident;
	}

	std::shared_ptr<Resident> Residency::pin(const std::shared_ptr<Mesh>& mesh)
	{
		{
			std::lock_guard<std::mutex> g(m);
			if (auto it = pinned.find(mesh.get()); it != pinned.end())
				return it->second;
		}

		auto resident = acquire(mesh);

		std::lock_guard<std::mutex> g(m);
		pinned[mesh.get()] = resident;
		return resident;
	}

	// A page can be shown only once the pages holding the groups that refine into it (its coarser parents) are:
	// otherwise the cut would find the fine groups and no stand-in for them.
	void Residency::find_parents(Resident& resident)
	{
		const Mesh& mesh = *resident.mesh;
		for (uint32_t q = 0; q < mesh.groups.size(); q++)
		{
			const Group& coarse = mesh.groups[q];
			GroupView view = read_group(mesh, q);
			for (uint32_t c = 0; c < coarse.cluster_count; c++)
			{
				const ClusterHeader& header = view.clusters[c];
				if (header.refined_error < 0)
					continue;

				const uint32_t first = family_first(header.refined_family);
				for (uint32_t i = 0; i < family_count(header.refined_family); i++)
				{
					const uint32_t fine_page = mesh.groups[first + i].page;
					if (fine_page == coarse.page)
						continue;
					auto& parents = resident.states[fine_page].parents;
					if (std::find(parents.begin(), parents.end(), coarse.page) == parents.end())
						parents.push_back(coarse.page);
				}
			}
		}
	}

	uint32_t Residency::take_slot()
	{
		const uint32_t slot = free_slots.back();
		free_slots.pop_back();
		return slot;
	}

	// Starts putting a page's bytes into its slot: through DirectStorage when the mesh came from an asset file
	// (done when the returned waiter is), otherwise as a queued upload (visible from the next prepare()).
	Residency::Loading Residency::upload_page(Resident& resident, uint32_t page, uint32_t slot, bool allow_file)
	{
		Loading loading_page = { &resident, page };
		const Page& source = resident.mesh->pages[page];

		// The slots' tiles must be mapped before DirectStorage writes them: the mapping happens on the queue.
		if (allow_file && g_direct_storage && pool_mapped && !resident.mesh->file.empty() && source.file_size != 0)
		{
			HAL::StorageRequest request;
			request.resource = pages.buffer.resource;
			request.file = resident.mesh->file;
			request.file_offset = source.file_offset;
			request.size = source.file_size;
			request.uncompressed_size = source.data.size();
			request.compressed = source.file_size < source.data.size();
			request.operation = HAL::StorageRequest::Buffer{ (pages.buffer.get_data_offset() + pool[slot].get_offset()) * sizeof(uint32_t) };

			auto& queue = RenderSystem::get().device().get_ds_queue();
			loading_page.waiter = queue.execute(request);
			loading_page.from_file = true;
			queue.flush();
			file_loads++;
			return loading_page;
		}

		std::vector<uint> dwords((source.data.size() + 3) / 4, 0);
		std::memcpy(dwords.data(), source.data.data(), source.data.size());
		pages.write(pool[slot].get_offset(), dwords);
		return loading_page;
	}

	void Residency::set_page_present(const Resident& resident, uint32_t page, bool present)
	{
		ASSERT(page < resident.mesh->pages.size());
		ASSERT(!resident.streamed);
		std::vector<uint> slot = { present ? static_cast<uint32_t>(resident.page_offset() + uint64_t(page) * PageDwords) : InvalidIndex };
		page_slots.write(resident.page_id_base() + page, slot);
	}

	StreamStats Residency::stats()
	{
		std::lock_guard<std::mutex> g(m);
		StreamStats out;
		out.pool_pages = static_cast<uint32_t>(pool.size());
		out.free_pages = static_cast<uint32_t>(free_slots.size());
		out.loading_pages = static_cast<uint32_t>(loading.size());
		out.loads = loads;
		out.file_loads = file_loads;
		out.evictions = evictions;
		out.snapshots = snapshots_done;
		out.wanted = last_wanted;
		return out;
	}

	void Residency::prepare(HAL::CommandList::ptr& list)
	{
		std::lock_guard<std::mutex> g(m);

		current_epoch++;
		pages.prepare(list);
		groups.prepare(list);
		nodes.prepare(list);
		page_slots.prepare(list);

		// The page-table writes of the changes below are in this list: draws recorded after it see the new cut.
		for (Resident* resident : changed)
			resident->generation.fetch_add(1, std::memory_order_relaxed);
		changed.clear();

		if (pool_map_pending)
		{
			// This list carries the pool's tile mappings; once it is done DirectStorage may write the slots.
			pool_map_pending = false;
			list->on_done([generation = pool_generation]
			{
				if (!Residency::is_good())
					return;
				auto& residency = Residency::get();
				std::lock_guard<std::mutex> g(residency.m);
				if (generation == residency.pool_generation)
					residency.pool_mapped = true;
			});
		}

		if (!retiring.empty())
		{
			// The table writes above hide these slots from every frame recorded after this list; the frames
			// already submitted may still be reading them, and they all finish before this list does.
			retiring_inflight += retiring.size();
			list->on_done([slots = std::move(retiring), generation = pool_generation]
			{
				if (!Residency::is_good())
					return;
				auto& residency = Residency::get();
				std::lock_guard<std::mutex> g(residency.m);
				if (generation != residency.pool_generation)
					return;   // the pool they came from was rebuilt
				residency.free_slots.insert(residency.free_slots.end(), slots.begin(), slots.end());
				residency.retiring_inflight -= slots.size();
			});
			retiring.clear();
		}

		if (streamer.joinable() && max_page_id > 0)
		{
			const uint32_t epoch = current_epoch - 1;   // the feedback holds the draws up to the previous frame
			list->get_copy().read_buffer(feedback.resource.get(), 0, uint64_t(max_page_id) * 2 * sizeof(uint32_t),
				[epoch](std::span<std::byte> bytes)
				{
					if (!Residency::is_good())
						return;
					auto& residency = Residency::get();
					Snapshot snapshot;
					snapshot.epoch = epoch;
					snapshot.data.resize(bytes.size() / sizeof(uint32_t));
					std::memcpy(snapshot.data.data(), bytes.data(), snapshot.data.size() * sizeof(uint32_t));
					{
						std::lock_guard<std::mutex> g(residency.snapshot_mutex);
						residency.snapshots.push_back(std::move(snapshot));
						if (residency.snapshots.size() > 2)
							residency.snapshots.pop_front();
					}
					residency.wake.notify_one();
				});
		}
	}

	void Residency::run_streamer()
	{
		for (;;)
		{
			Snapshot snapshot;
			bool have_snapshot = false;
			{
				// Wakes without a snapshot too, to show pages whose DirectStorage reads have finished.
				std::unique_lock<std::mutex> lock(snapshot_mutex);
				wake.wait_for(lock, std::chrono::milliseconds(2), [&] { return stopping || !snapshots.empty(); });
				if (stopping)
					return;
				if (!snapshots.empty())
				{
					snapshot = std::move(snapshots.front());
					snapshots.pop_front();
					have_snapshot = true;
				}
			}

			if (have_snapshot)
			{
				process(snapshot);
			}
			else
			{
				std::lock_guard<std::mutex> g(m);
				if (!loading.empty())
					publish();
			}
		}
	}

	// One feedback snapshot: refresh the LRU stamps, then start loading what the draw wanted and lacked.
	void Residency::process(const Snapshot& snapshot)
	{
		PROFILE(L"quanta_streamer");
		std::lock_guard<std::mutex> g(m);

		const uint32_t epoch16 = snapshot.epoch & 0xFFFF;
		const size_t count = std::min<size_t>(snapshot.data.size() / 2, page_refs.size());

		struct Wanted
		{
			uint32_t id;
			uint32_t priority;   // the half-float error bits: positive halves order like their values
		};
		std::vector<Wanted> wanted;

		for (size_t id = 0; id < count; id++)
		{
			const PageRef& ref = page_refs[id];
			if (!ref.resident)
				continue;

			PageState& state = ref.resident->states[ref.page];
			const uint32_t touched = snapshot.data[id * 2 + 1];
			if (state.status == PageState::Status::Resident && touched > state.last_touched)
				state.last_touched = touched;

			const uint32_t want = snapshot.data[id * 2];
			if (want != 0 && (want >> 16) == epoch16 && state.status == PageState::Status::Missing)
				wanted.push_back({ static_cast<uint32_t>(id), want & 0xFFFF });
		}
		last_wanted = static_cast<uint32_t>(wanted.size());
		snapshots_done++;

		std::sort(wanted.begin(), wanted.end(), [](const Wanted& a, const Wanted& b) { return a.priority > b.priority; });

		// Each wanted page with its missing ancestors ahead of it.
		std::vector<PageRef> order;
		std::unordered_set<uint32_t> queued;
		auto add = [&](auto&& self, Resident& resident, uint32_t page) -> void
		{
			if (resident.states[page].status != PageState::Status::Missing)
				return;
			if (!queued.insert(static_cast<uint32_t>(resident.page_id_base() + page)).second)
				return;
			for (uint32_t parent : resident.states[page].parents)
				self(self, resident, parent);
			order.push_back({ &resident, page });
		};
		for (const Wanted& w : wanted)
		{
			if (order.size() >= MaxLoadsPerSnapshot)
				break;
			add(add, *page_refs[w.id].resident, page_refs[w.id].page);
		}

		// Slots already on their way back (hidden, waiting for their frames to finish) count as available.
		const size_t available = free_slots.size() + retiring.size() + retiring_inflight;
		if (order.size() > available)
			evict(order.size() - available, snapshot.epoch);

		for (const PageRef& ref : order)
		{
			if (free_slots.empty())
				break;

			PageState& state = ref.resident->states[ref.page];
			state.slot = take_slot();
			state.status = PageState::Status::Loading;
			loading.push_back(upload_page(*ref.resident, ref.page, state.slot, true));
			loads++;
		}

		publish();
	}

	// Shows every loaded page whose parents are shown, parents first.
	void Residency::publish()
	{
		bool progress = true;
		while (progress)
		{
			progress = false;
			for (size_t i = 0; i < loading.size();)
			{
				Loading item = loading[i];
				PageState& state = item.resident->states[item.page];

				if (item.from_file && !item.waiter.is_completed())
				{
					i++;
					continue;
				}

				bool parents_shown = true;
				for (uint32_t parent : state.parents)
					parents_shown = parents_shown && item.resident->states[parent].status == PageState::Status::Resident;

				if (!parents_shown)
				{
					i++;
					continue;
				}

				std::vector<uint> slot = { static_cast<uint32_t>(pool[state.slot].get_offset()) };
				page_slots.write(item.resident->page_id_base() + item.page, slot);
				state.status = PageState::Status::Resident;
				state.last_touched = current_epoch;
				changed.insert(item.resident);
				for (uint32_t parent : state.parents)
					item.resident->states[parent].resident_children++;

				loading.erase(loading.begin() + i);
				progress = true;
			}
		}
	}

	// Hides up to `count` pages, least recently reached first, to make room. Their slots return to the pool once
	// the frames that may still read them are done (prepare).
	void Residency::evict(size_t count, uint32_t epoch)
	{
		struct Victim
		{
			uint32_t last_touched;
			PageRef ref;
		};
		std::vector<Victim> victims;

		for (Resident* resident : streamed_list)
			for (uint32_t p = 0; p < resident->states.size(); p++)
			{
				const PageState& state = resident->states[p];
				if (state.status == PageState::Status::Resident && !resident->mesh->pages[p].root
					&& state.resident_children == 0 && state.last_touched + EvictionAge < epoch)
					victims.push_back({ state.last_touched, { resident, p } });
			}

		std::sort(victims.begin(), victims.end(), [](const Victim& a, const Victim& b) { return a.last_touched < b.last_touched; });

		for (size_t i = 0; i < std::min(count, victims.size()); i++)
		{
			Resident& resident = *victims[i].ref.resident;
			PageState& state = resident.states[victims[i].ref.page];

			std::vector<uint> hidden = { InvalidIndex };
			page_slots.write(resident.page_id_base() + victims[i].ref.page, hidden);
			changed.insert(&resident);
			for (uint32_t parent : state.parents)
				resident.states[parent].resident_children--;

			retiring.push_back(state.slot);
			state.slot = InvalidIndex;
			state.status = PageState::Status::Missing;
			evictions++;
		}
	}
}
