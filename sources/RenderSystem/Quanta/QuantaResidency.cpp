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
	Variable<int> g_pool_pages = { 1024, "Pool pages", &streaming_context(), 2, 4096 };

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
	}

	void Residency::allocate_pool(uint32_t count)
	{
		const size_t first = pool.size();
		pool.resize(first + count);
		for (size_t i = first; i < pool.size(); i++)
			pages.allocate(pool[i], PageDwords);
	}

	std::shared_ptr<Resident> Residency::acquire(const std::shared_ptr<Mesh>& mesh)
	{
		std::lock_guard<std::mutex> g(m);

		if (auto it = residents.find(mesh.get()); it != residents.end())
			if (auto existing = it->second.lock())
				return existing;

		const bool stream = g_streaming;

		{ // TEMP: scene size log (quanta_meshes.temp)
			static uint64_t total_triangles = 0, total_pages = 0;
			static uint32_t mesh_count = 0;
			total_triangles += mesh->original_triangle_count;
			total_pages += mesh->pages.size();
			std::ofstream("quanta_meshes.temp", std::ios::app) << "MESH #" << ++mesh_count << " streamed=" << stream << " triangles=" << mesh->original_triangle_count
				<< " pages=" << mesh->pages.size() << " (" << mesh->pages.size() / 4.0 << " MB) groups=" << mesh->groups.size() << " nodes=" << mesh->nodes.size()
				<< " | total triangles=" << total_triangles << " pages=" << total_pages << " (" << total_pages / 4.0 << " MB)\n";
		}

		auto resident = std::make_shared<Resident>();
		resident->mesh = mesh;
		resident->streamed = stream;

		// The pool holds every page of the pinned meshes, and the budget the streamed ones share.
		if (stream)
			streamed_meshes++;
		else
			pinned_pages += static_cast<uint32_t>(mesh->pages.size());
		const uint32_t budget = static_cast<uint32_t>(std::max(2, int(g_pool_pages)));
		streamer->set_budget(budget);
		streamer->ensure_pool(std::max(2u, pinned_pages + (streamed_meshes ? budget : 0)));

		// The roots go into slots right below.
		uint32_t root_pages = 0;
		for (uint32_t p = 0; p < mesh->pages.size(); p++)
			root_pages += resident->is_root(p);
		streamer->ensure_free(root_pages);

		page_slots.allocate(resident->page_id_handle, mesh->pages.size());
		groups.allocate(resident->group_handle, mesh->groups.size() * GroupDwords);
		nodes.allocate(resident->node_handle, mesh->nodes.size() * NodeDwords);

		const uint64_t group_base = resident->group_handle.get_offset() / GroupDwords;
		ASSERT(resident->group_handle.get_offset() % GroupDwords == 0 && resident->node_handle.get_offset() % NodeDwords == 0);

		const uint64_t page_id_base = resident->page_id_handle.get_offset();
		ASSERT(page_id_base + mesh->pages.size() <= MaxPageIds);
		max_page_id = std::max<uint32_t>(max_page_id, static_cast<uint32_t>(page_id_base + mesh->pages.size()));

		// The ids may have belonged to another mesh: nothing is loading or evictable until the streamer says so.
		flag_writes.push_back({ static_cast<uint32_t>(page_id_base), std::vector<uint32_t>(mesh->pages.size() * 2, 0) });

		std::vector<uint> slots(mesh->pages.size(), InvalidIndex);
		resident->states.resize(mesh->pages.size());
		if (stream)
			find_parents(*resident);
		streamer->add(*resident);

		// Roots are always there: they are what a missing page falls back to. A pinned mesh has nothing but roots.
		for (uint32_t p = 0; p < mesh->pages.size(); p++)
			if (resident->is_root(p))
				slots[p] = static_cast<uint32_t>(pool[streamer->load_root(*resident, p)].get_offset());
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

		if (stream)
			streamer->start();

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

	// Starts putting a page's bytes into its slot: through DirectStorage when the mesh came from an asset file
	// (done when the returned waiter is), otherwise as a queued upload (visible from the next prepare()).
	Streaming::Backend::Load Residency::begin_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file)
	{
		Resident& resident = static_cast<Resident&>(resource);
		Streaming::Backend::Load loading_page;
		const Page& source = resident.mesh->pages[page];

		// The slots' memory must be ready before DirectStorage writes it: the mapping happens on the queue.
		if (allow_file && g_direct_storage && streamer->pool_mapped() && !resident.mesh->file.empty() && source.file_size != 0)
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
			return loading_page;
		}

		std::vector<uint> dwords((source.data.size() + 3) / 4, 0);
		std::memcpy(dwords.data(), source.data.data(), source.data.size());
		pages.write(pool[slot].get_offset(), dwords);
		return loading_page;
	}

	void Residency::show(Streaming::Resource& resource, uint32_t page, uint32_t slot)
	{
		std::vector<uint> offset = { static_cast<uint32_t>(pool[slot].get_offset()) };
		page_slots.write(resource.page_id_base() + page, offset);
	}

	void Residency::hide(Streaming::Resource& resource, uint32_t page)
	{
		std::vector<uint> hidden = { InvalidIndex };
		page_slots.write(resource.page_id_base() + page, hidden);
	}

	void Residency::set_flags(Streaming::Resource& resource, uint32_t page, uint32_t flags, uint32_t shown_epoch)
	{
		flag_writes.push_back({ static_cast<uint32_t>(resource.page_id_base() + page), { flags, shown_epoch } });
	}

	void Residency::set_page_present(const Resident& resident, uint32_t page, bool present)
	{
		ASSERT(page < resident.mesh->pages.size());
		ASSERT(!resident.streamed);
		std::vector<uint> slot = { present ? static_cast<uint32_t>(page_dword_offset(resident, page)) : InvalidIndex };
		page_slots.write(resident.page_id_base() + page, slot);
	}

	uint64_t Residency::page_dword_offset(const Resident& resident, uint32_t page)
	{
		std::lock_guard<std::mutex> g(m);
		ASSERT(resident.states[page].slot != Streaming::InvalidSlot);
		return pool[resident.states[page].slot].get_offset();
	}

	StreamStats Residency::stats()
	{
		return streamer->stats();
	}

	void Residency::prepare(HAL::CommandList::ptr& list)
	{
		std::lock_guard<std::mutex> g(m);

		streamer->advance_epoch();
		streamer->set_budget(static_cast<uint32_t>(std::max(2, int(g_pool_pages))));
		pages.prepare(list);
		groups.prepare(list);
		nodes.prepare(list);
		page_slots.prepare(list);

		for (FlagWrite& write : flag_writes)
			list->get_copy().update(page_flags, write.first * 2, std::span{ write.dwords });
		flag_writes.clear();

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
