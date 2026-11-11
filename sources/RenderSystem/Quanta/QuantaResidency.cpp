module Graphics:QuantaResidency;

import :QuantaResidency;

import Core;
import HAL;
import :Quanta;
import :Streaming;

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
	Variable<int> g_pool_pages = { 1024, "Pool pages", &streaming_context(), 2, 16000 };

	// Pages of a mesh loaded from an asset file are read straight into their pool slot by DirectStorage.
	Variable<bool> g_direct_storage = { true, "Direct storage", &streaming_context() };

	// The layout of the buffer the GPU compacts the feedback into; must match shaders/streaming/stream_compact.hlsl.
	constexpr uint32_t RequestBuckets = 16;
	constexpr uint32_t BucketCapacity = 32;
	constexpr uint32_t EvictCapacity = 128;
	constexpr uint32_t OutRequestOverflow = 16;
	constexpr uint32_t OutEvictCount = 17;
	constexpr uint32_t OutRequests = 32;
	constexpr uint32_t OutEvicts = OutRequests + RequestBuckets * BucketCapacity * 2;
	constexpr uint32_t OutDwords = OutEvicts + EvictCapacity * 2;
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
		: streamer(std::make_shared<Streaming::Streamer>(static_cast<Streaming::Backend&>(*this), static_cast<uint32_t>(MaxPageIds)))
		, m(streamer->mutex())
		, pages(RenderSystem::get().device(), MaxPageDwords)
		, groups(RenderSystem::get().device(), MaxGroupDwords)
		, nodes(RenderSystem::get().device(), MaxNodeDwords)
		, page_slots(RenderSystem::get().device(), MaxPageIds)
		, feedback(RenderSystem::get().device(), MaxPageIds * 2, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)
		, page_flags(RenderSystem::get().device(), MaxPageIds * 2, HAL::counterType::NONE, HAL::ResFlags::ShaderResource)
		, stream_out(RenderSystem::get().device(), OutDwords, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)
	{
	}

	Residency::~Residency()
	{
		// Before any member goes: the streamer thread calls back into this object.
		streamer->stop();
	}

	Resident::~Resident()
	{
		if (!Residency::is_good())
			return;

		auto& residency = Residency::get();
		residency.release(*this);
		residency.page_slots.free(page_id_handle);
		residency.groups.free(group_handle);
		residency.nodes.free(node_handle);
	}

	void Residency::release(Resident& resident)
	{
		std::lock_guard<std::mutex> g(m);
		streamer->remove(resident);

		if (resident.streamed)
			streamed_meshes--;
		else
			pinned_pages -= static_cast<uint32_t>(resident.states.size());

		page_ids_in_use -= resident.mesh->pages.size();
		groups_in_use -= resident.mesh->groups.size();
		nodes_in_use -= resident.mesh->nodes.size();
	}

	uint32_t Residency::streaming_budget() const
	{
		const uint64_t held = uint64_t(pinned_pages) + streamer->root_slots();
		const uint32_t room = held < MaxPoolPages ? static_cast<uint32_t>(MaxPoolPages - held) : 0;
		const uint32_t wanted = static_cast<uint32_t>(std::max(2, int(g_pool_pages)));
		return std::max(2u, std::min(wanted, room));
	}

	void Residency::allocate_pool(uint32_t count)
	{
		if (!count)
			return;

		PROFILE(L"quanta_allocate_pool");
		pages.allocate(pool_ranges.emplace_back(), size_t(count) * PageDwords);
		const uint64_t first = pool_ranges.back().get_offset();
		for (uint32_t i = 0; i < count; i++)
			pool.push_back(first + uint64_t(i) * PageDwords);
	}

	std::shared_ptr<Resident> Residency::acquire(const std::shared_ptr<Mesh>& mesh)
	{
		PROFILE(L"quanta_acquire");
		std::lock_guard<std::mutex> g(m);

		if (auto it = residents.find(mesh.get()); it != residents.end())
			if (auto existing = it->second.lock())
				return existing;

		const bool stream = g_streaming;

		// Refused before anything changes, so the scene can go on without this mesh.
		auto refuse = [&](const char* what, uint64_t need, uint64_t used, uint64_t cap, const char* hint)
		{
			throw std::runtime_error(std::string("Quanta: out of ") + what + ": the mesh needs " + std::to_string(need) + ", "
				+ std::to_string(used) + " of " + std::to_string(cap) + " are in use. " + hint);
		};
		if (page_ids_in_use + mesh->pages.size() > MaxPageIds)
			refuse("page ids", mesh->pages.size(), page_ids_in_use, MaxPageIds, "Raise MaxPageIds in QuantaResidency.ixx.");
		if (groups_in_use + mesh->groups.size() > MaxGroups)
			refuse("groups", mesh->groups.size(), groups_in_use, MaxGroups, "The family index packs 24 bits (FamilyCountShift).");
		if (nodes_in_use + mesh->nodes.size() > MaxNodes)
			refuse("nodes", mesh->nodes.size(), nodes_in_use, MaxNodes, "Raise MaxNodes in QuantaResidency.ixx.");
		if (!stream && pinned_pages + mesh->pages.size() > MaxPoolPages)
			refuse("pinned pages", mesh->pages.size(), pinned_pages, MaxPoolPages, "Turn Streaming on so only the pages in view are resident.");

		page_ids_in_use += mesh->pages.size();
		groups_in_use += mesh->groups.size();
		nodes_in_use += mesh->nodes.size();

		auto resident = std::make_shared<Resident>();
		resident->mesh = mesh;
		resident->streamed = stream;

		// The pool holds every page of the pinned meshes, and the budget the streamed ones share.
		if (stream)
			streamed_meshes++;
		else
			pinned_pages += static_cast<uint32_t>(mesh->pages.size());
		// The roots go into slots right below.
		uint32_t root_pages = 0;
		for (uint32_t p = 0; p < mesh->pages.size(); p++)
			root_pages += resident->is_root(p);

		const uint32_t budget = streaming_budget();
		streamer->set_budget(budget);
		{
			PROFILE(L"quanta_acquire_grow_pool");
			// A streamed mesh's roots count in root_slots() only once loaded, below; a pinned one's are in pinned_pages.
			const uint32_t streamed_roots = streamer->root_slots() + (stream ? root_pages : 0);
			streamer->ensure_pool(std::max(2u, pinned_pages + (streamed_meshes ? streamed_roots + budget : 0)));
			streamer->ensure_free(root_pages);
		}

		page_slots.allocate(resident->page_id_handle, mesh->pages.size());
		groups.allocate(resident->group_handle, mesh->groups.size() * GroupDwords);
		nodes.allocate(resident->node_handle, mesh->nodes.size() * NodeDwords);

		const uint64_t group_base = resident->group_handle.get_offset() / GroupDwords;
		ASSERT(resident->group_handle.get_offset() % GroupDwords == 0 && resident->node_handle.get_offset() % NodeDwords == 0);

		const uint64_t page_id_base = resident->page_id_handle.get_offset();
		ASSERT(page_id_base + mesh->pages.size() <= MaxPageIds);
		max_page_id = std::max<uint32_t>(max_page_id, static_cast<uint32_t>(page_id_base + mesh->pages.size()));

		// The ids may have belonged to another mesh: nothing is loading or evictable until the streamer says so.
		{
			const uint32_t first_id = static_cast<uint32_t>(page_id_base);
			const uint32_t end_id = first_id + static_cast<uint32_t>(mesh->pages.size());
			std::erase_if(flag_singles, [&](const auto& single) { return single.first >= first_id && single.first < end_id; });
			flag_writes.push_back({ first_id, std::vector<uint32_t>(mesh->pages.size() * 2, 0) });
		}

		std::vector<uint> slots(mesh->pages.size(), InvalidIndex);
		resident->states.resize(mesh->pages.size());
		if (stream)
			find_parents(*resident);
		streamer->add(*resident);

		// Roots are always there: they are what a missing page falls back to. A pinned mesh has nothing but roots.
		{
			PROFILE(L"quanta_acquire_load_roots");
			for (uint32_t p = 0; p < mesh->pages.size(); p++)
				if (resident->is_root(p))
					slots[p] = static_cast<uint32_t>(pool[streamer->load_root(*resident, p)]);
		}
		page_slots.write(page_id_base, std::move(slots));

		std::vector<uint> group_dwords(mesh->groups.size() * GroupDwords);
		for (size_t i = 0; i < mesh->groups.size(); i++)
		{
			Group record = mesh->groups[i];
			record.page = static_cast<uint32_t>(page_id_base + record.page);
			record.family = pack_family(static_cast<uint32_t>(group_base + family_first(record.family)), family_count(record.family));
			std::memcpy(&group_dwords[i * GroupDwords], &record, sizeof(Group));
		}
		groups.write(resident->group_handle.get_offset(), std::move(group_dwords));

		std::vector<uint> node_dwords(mesh->nodes.size() * NodeDwords);
		for (size_t i = 0; i < mesh->nodes.size(); i++)
		{
			Node record = mesh->nodes[i];
			record.first_group = static_cast<uint32_t>(group_base + record.first_group);
			std::memcpy(&node_dwords[i * NodeDwords], &record, sizeof(Node));
		}
		nodes.write(resident->node_handle.get_offset(), std::move(node_dwords));

		if (stream)
			streamer->start();

		residents[mesh.get()] = resident;
		return resident;
	}

	std::vector<std::shared_ptr<Resident>> Residency::acquire_many(const std::vector<std::shared_ptr<Mesh>>& meshes)
	{
		PROFILE(L"quanta_acquire_many");

		std::vector<std::shared_ptr<Resident>> result(meshes.size());

		// One pass for the whole set: the root pages read ahead are what the pool grows by anyway, so holding them
		// until the meshes are registered costs no memory the pool would not take.
		std::vector<PageFetch> fetches;
		uint32_t new_roots = 0;
		const bool stream = streaming_enabled();

		{
			PROFILE(L"quanta_acquire_many_scan");
			std::lock_guard<std::mutex> g(m);
			std::unordered_set<const Mesh*> seen;
			for (const auto& shared : meshes)
			{
				const Mesh& mesh = *shared;
				if (!seen.insert(&mesh).second)
					continue;
				if (auto it = residents.find(&mesh); it != residents.end() && !it->second.expired())
					continue;

				for (uint32_t p = 0; p < mesh.pages.size(); p++)
					if (!stream || mesh.pages[p].root)
					{
						new_roots++;
						if (mesh.pages[p].data.empty() && mesh.pages[p].size != 0)
							fetches.push_back({ &mesh, p });
					}
			}
		}

		{
			PROFILE(L"quanta_prefetch_roots");

			// The meshes of one asset share its file: one read task per file, which opens it once and reads in file order.
			std::map<std::filesystem::path, std::vector<PageFetch*>> by_file;
			for (auto& fetch : fetches)
				if (!fetch.mesh->file.empty())
					by_file[fetch.mesh->file].push_back(&fetch);

			std::vector<std::future<void>> tasks;
			tasks.reserve(by_file.size());
			for (auto& entry : by_file)
				tasks.emplace_back(thread_pool::get().enqueue(TaskOptions{ TaskPriority::HIGH }, [&entry]()
				{
					read_stored_pages(entry.first, entry.second);
				}));
			for (auto& task : tasks)
				task.wait();

			// Decompression is CPU work and fans out; pages stored raw are done.
			tasks.clear();
			for (auto& fetch : fetches)
				if (fetch.ok && fetch.mesh->pages[fetch.page].file_size != fetch.mesh->pages[fetch.page].size)
					tasks.emplace_back(thread_pool::get().enqueue(TaskOptions{ TaskPriority::HIGH }, [&fetch]()
					{
						expand_page(fetch);
					}));
			for (auto& task : tasks)
				task.wait();
		}

		{
			std::lock_guard<std::mutex> g(m);

			prefetched.clear();
			for (auto& fetch : fetches)
				if (fetch.ok)
					prefetched.push_back(std::move(fetch));
			std::sort(prefetched.begin(), prefetched.end(), [](const PageFetch& a, const PageFetch& b)
			{
				return std::make_pair(a.mesh, a.page) < std::make_pair(b.mesh, b.page);
			});

			// The pool for the whole set at once: acquire() below then finds it already large enough, instead of
			// growing it by a page or two for each mesh.
			if (new_roots)
			{
				PROFILE(L"quanta_acquire_many_grow_pool");
				if (stream)
				{
					const uint32_t budget = streaming_budget();
					streamer->set_budget(budget);
					streamer->ensure_pool(std::max(2u, pinned_pages + streamer->root_slots() + new_roots + budget));
				}
				else
					streamer->ensure_pool(std::max(2u, pinned_pages + new_roots));
				streamer->ensure_free(new_roots);
			}
		}

		{
			PROFILE(L"quanta_acquire_many_register");
			for (size_t i = 0; i < meshes.size(); i++)
			{
				try
				{
					result[i] = acquire(meshes[i]);
				}
				catch (const std::exception& e)
				{
					Log::get() << e.what() << Log::endl;
				}
			}
		}

		std::lock_guard<std::mutex> g(m);
		prefetched.clear();
		return result;
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
	// otherwise the cut would find the fine groups and no stand-in for them. The table is part of the mesh.
	void Residency::find_parents(Resident& resident)
	{
		Mesh& mesh = *resident.mesh;
		ASSERT(mesh.parents_built);   // Pack and deserialize fill it
		for (uint32_t p = 0; p < mesh.pages.size(); p++)
			resident.states[p].parents = mesh.pages[p].parents;
	}

	namespace
	{
		// What plan_load hands start_loads for a page that is not already in memory.
		struct PagePlan
		{
			std::shared_ptr<Mesh> mesh;                 // read after its Resident may be gone
			uint32_t page = 0;
			uint64_t dword_offset = 0;                  // of the slot in `pages`, taken under the lock
			std::optional<HAL::StorageRequest> request; // read by DirectStorage straight into the slot
			std::vector<uint8_t> bytes;                 // otherwise read here by start_loads, queued by finish_loads
		};
	}

	void Residency::write_page(uint64_t dword_offset, const std::vector<uint8_t>& bytes)
	{
		std::vector<uint> dwords((bytes.size() + 3) / 4, 0);
		std::memcpy(dwords.data(), bytes.data(), bytes.size());
		pages.write(dword_offset, std::move(dwords));
	}

	// Where a page's bytes come from: DirectStorage straight into the slot when the mesh came from an asset file (done
	// when the waiter start_loads gives the plan is), otherwise a queued upload (visible from the next prepare()), of
	// bytes in memory now or read from the file by start_loads.
	Streaming::Backend::LoadPlan Residency::plan_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file)
	{
		Resident& resident = static_cast<Resident&>(resource);
		Streaming::Backend::LoadPlan plan;
		const Page& source = resident.mesh->pages[page];
		const uint64_t dword_offset = pool[slot];

		// The slots' memory must be ready before DirectStorage writes it: the mapping happens on the queue.
		if (allow_file && g_direct_storage && streamer->pool_mapped() && !resident.mesh->file.empty() && source.file_size != 0)
		{
			auto data = std::make_shared<PagePlan>();
			data->mesh = resident.mesh;
			data->page = page;
			data->dword_offset = dword_offset;

			HAL::StorageRequest& request = data->request.emplace();
			request.resource = pages.buffer.resource;
			request.file = resident.mesh->file;
			request.file_offset = source.file_offset;
			request.size = source.file_size;
			request.uncompressed_size = source.bytes();
			request.compressed = source.file_size < source.bytes();
			request.operation = HAL::StorageRequest::Buffer{ (pages.buffer.get_data_offset() + dword_offset) * sizeof(uint32_t) };

			plan.from_file = true;
			plan.backend_data = std::move(data);
			return plan;
		}

		// Not in memory for a loaded asset: read (and expanded) from its file by start_loads, unless acquire_many() read it ahead.
		if (source.data.empty() && source.size != 0)
		{
			const auto key = std::make_pair(static_cast<const Mesh*>(resident.mesh.get()), page);
			const auto ahead = std::lower_bound(prefetched.begin(), prefetched.end(), key,
				[](const PageFetch& fetch, const std::pair<const Mesh*, uint32_t>& k) { return std::make_pair(fetch.mesh, fetch.page) < k; });
			if (ahead != prefetched.end() && ahead->mesh == key.first && ahead->page == page)
			{
				write_page(dword_offset, ahead->bytes);
				return plan;
			}

			auto data = std::make_shared<PagePlan>();
			data->mesh = resident.mesh;
			data->page = page;
			data->dword_offset = dword_offset;
			plan.backend_data = std::move(data);
			return plan;
		}

		write_page(dword_offset, source.data);
		return plan;
	}

	void Residency::start_loads(std::span<Streaming::Backend::LoadPlan> plans)
	{
		std::vector<HAL::StorageRequest> requests;
		std::vector<Streaming::Backend::LoadPlan*> direct;

		for (auto& plan : plans)
		{
			auto* data = static_cast<PagePlan*>(plan.backend_data.get());
			if (!data)
				continue;

			if (data->request)
			{
				requests.push_back(*data->request);
				direct.push_back(&plan);
				continue;
			}

			PROFILE(L"quanta_read_page");
			if (!read_page(*data->mesh, data->page, data->bytes))
			{
				// Runs on the streamer thread too: an empty page is better than a thrown exception there.
				Log::get() << "Quanta: cannot read page " << data->page << " from " << data->mesh->file.string() << Log::endl;
				data->bytes.assign(data->mesh->pages[data->page].size, 0);
			}
		}

		if (!requests.empty())
		{
			PROFILE(L"quanta_direct_storage");
			// One signal and one submit for the batch: every page of it is done together.
			const HAL::FenceWaiter waiter = RenderSystem::get().device().get_ds_queue().execute(std::span<const HAL::StorageRequest>(requests));
			for (auto* plan : direct)
				plan->waiter = waiter;
		}
	}

	void Residency::finish_loads(std::span<Streaming::Backend::LoadPlan> plans)
	{
		for (auto& plan : plans)
		{
			auto* data = static_cast<PagePlan*>(plan.backend_data.get());
			if (!data || data->request)
				continue;

			write_page(data->dword_offset, data->bytes);
			plan.backend_data.reset();   // the bytes are queued: nothing left to keep
		}
	}

	void Residency::show(Streaming::Resource& resource, uint32_t page, uint32_t slot)
	{
		const uint offset = static_cast<uint32_t>(pool[slot]);
		page_slots.write(resource.page_id_base() + page, std::span<const uint>(&offset, 1));
	}

	void Residency::hide(Streaming::Resource& resource, uint32_t page)
	{
		const uint hidden = InvalidIndex;
		page_slots.write(resource.page_id_base() + page, std::span<const uint>(&hidden, 1));
	}

	void Residency::set_flags(Streaming::Resource& resource, uint32_t page, uint32_t flags, uint32_t shown_epoch)
	{
		flag_singles.push_back({ static_cast<uint32_t>(resource.page_id_base() + page), { flags, shown_epoch } });
	}

	void Residency::set_page_present(const Resident& resident, uint32_t page, bool present)
	{
		ASSERT(page < resident.mesh->pages.size());
		ASSERT(!resident.streamed);
		const uint slot = present ? static_cast<uint32_t>(page_dword_offset(resident, page)) : InvalidIndex;
		page_slots.write(resident.page_id_base() + page, std::span<const uint>(&slot, 1));
	}

	uint64_t Residency::page_dword_offset(const Resident& resident, uint32_t page)
	{
		std::lock_guard<std::mutex> g(m);
		ASSERT(resident.states[page].slot != Streaming::InvalidSlot);
		return pool[resident.states[page].slot];
	}

	StreamStats Residency::stats()
	{
		return streamer->stats();
	}

	void Residency::prepare(HAL::CommandList::ptr& list)
	{
		std::lock_guard<std::mutex> g(m);

		streamer->advance_epoch();
		streamer->set_budget(streaming_budget());
		pages.prepare(list);
		groups.prepare(list);
		nodes.prepare(list);
		page_slots.prepare(list);

		if (!flag_writes.empty() || !flag_singles.empty())
		{
			PROFILE(L"quanta_flag_writes");

			// Ranges first, then the singles recorded after them; for the same page id the last single wins, so keep only that.
			std::stable_sort(flag_singles.begin(), flag_singles.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
			size_t kept = 0;
			for (size_t i = 0; i < flag_singles.size(); i++)
				if (i + 1 == flag_singles.size() || flag_singles[i + 1].first != flag_singles[i].first)
					flag_singles[kept++] = flag_singles[i];
			flag_singles.resize(kept);

			const uint64_t base = page_flags.desc.offset;
			std::vector<HAL::CopyContext::BufferWrite> writes;
			writes.reserve(flag_writes.size() + flag_singles.size());
			for (const FlagWrite& write : flag_writes)
				writes.push_back({ base + uint64_t(write.first) * 2 * sizeof(uint32_t), std::as_bytes(std::span<const uint32_t>(write.dwords)) });
			for (const auto& single : flag_singles)
				writes.push_back({ base + uint64_t(single.first) * 2 * sizeof(uint32_t), std::as_bytes(std::span<const uint32_t>(single.second)) });
			list->get_copy().update_buffer_batch(page_flags.resource.get(), writes);

			flag_writes.clear();
			flag_singles.clear();
		}

		streamer->prepare(list);

		if (streamer->running() && max_page_id > 0)
		{
			const uint32_t epoch = streamer->epoch() - 1;   // the feedback holds the draws up to the previous frame

			// The feedback stays on the GPU: one thread per page id picks out the pages that were wanted and are
			// missing, and the idle ones that can go, and only those lists are read back.
			auto& compute = list->get_compute();
			compute.set_signature(Layouts::DefaultLayout);
			compute.clear(stream_out);

			Slots::StreamGpu::StreamCompact compact;
			compact.GetRequests() = feedback.structuredBuffer;
			compact.GetPage_slots() = page_slots.buffer.structuredBuffer;
			compact.GetPage_flags() = page_flags.structuredBuffer;
			compact.GetResult() = stream_out;
			compact.GetPage_count() = max_page_id;
			compact.GetEpoch() = epoch;
			compact.GetEviction_age() = Streaming::Policy{}.eviction_age;
			compute.set(compact);
			compute.set_pipeline<PSOS::StreamGpu::StreamCompact>();
			compute.dispatch((int)Math::DivideByMultiple(max_page_id, 64u), 1, 1);

			list->get_copy().read_buffer(stream_out.resource.get(), 0, uint64_t(OutDwords) * sizeof(uint32_t),
				[epoch](std::span<std::byte> bytes)
				{
					if (!Residency::is_good() || bytes.size() < OutDwords * sizeof(uint32_t))
						return;

					const uint32_t* data = reinterpret_cast<const uint32_t*>(bytes.data());

					Streaming::Requests requests;
					requests.epoch = epoch;
					requests.dropped = data[OutRequestOverflow];
					for (uint32_t bucket = 0; bucket < RequestBuckets; bucket++)
						for (uint32_t i = 0, n = std::min(data[bucket], BucketCapacity); i < n; i++)
						{
							const uint32_t entry = OutRequests + (bucket * BucketCapacity + i) * 2;
							requests.wanted.push_back({ data[entry], data[entry + 1] });
						}
					for (uint32_t i = 0, n = std::min(data[OutEvictCount], EvictCapacity); i < n; i++)
					{
						const uint32_t entry = OutEvicts + i * 2;
						requests.victims.push_back({ data[entry], data[entry + 1] });
					}
					Residency::get().streamer->submit(std::move(requests));
				});
		}
	}
}
