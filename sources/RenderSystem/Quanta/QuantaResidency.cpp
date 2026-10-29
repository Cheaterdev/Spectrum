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
		if (!streamed)
			residency.pages.free(page_handle);
		residency.page_slots.free(page_id_handle);
		residency.groups.free(group_handle);
		residency.nodes.free(node_handle);
	}

	void Residency::release(Resident& resident)
	{
		std::lock_guard<std::mutex> g(m);
		streamer->remove(resident);
	}

	void Residency::allocate_pool(uint32_t count)
	{
		pool.resize(count);
		for (uint32_t i = 0; i < count; i++)
			pages.allocate(pool[i], PageDwords);
	}

	void Residency::free_pool()
	{
		for (auto& handle : pool)
			pages.free(handle);
		pool.clear();
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
			streamer->ensure_pool(static_cast<uint32_t>(std::max(2, int(g_pool_pages))));
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
			find_parents(*resident);
			streamer->add(*resident);

			// Roots are always there: they are what a missing page falls back to.
			for (uint32_t p = 0; p < mesh->pages.size(); p++)
				if (mesh->pages[p].root)
					slots[p] = static_cast<uint32_t>(pool[streamer->load_root(*resident, p)].get_offset());
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

	void Residency::set_page_present(const Resident& resident, uint32_t page, bool present)
	{
		ASSERT(page < resident.mesh->pages.size());
		ASSERT(!resident.streamed);
		std::vector<uint> slot = { present ? static_cast<uint32_t>(resident.page_offset() + uint64_t(page) * PageDwords) : InvalidIndex };
		page_slots.write(resident.page_id_base() + page, slot);
	}

	StreamStats Residency::stats()
	{
		return streamer->stats();
	}

	void Residency::prepare(HAL::CommandList::ptr& list)
	{
		std::lock_guard<std::mutex> g(m);

		streamer->advance_epoch();
		pages.prepare(list);
		groups.prepare(list);
		nodes.prepare(list);
		page_slots.prepare(list);
		streamer->prepare(list);

		if (streamer->running() && max_page_id > 0)
		{
			const uint32_t epoch = streamer->epoch() - 1;   // the feedback holds the draws up to the previous frame
			list->get_copy().read_buffer(feedback.resource.get(), 0, uint64_t(max_page_id) * 2 * sizeof(uint32_t),
				[epoch](std::span<std::byte> bytes)
				{
					if (!Residency::is_good())
						return;

					// Two dwords per page id: the latest epoch and half-float error it was wanted at, and the last
					// epoch a resident page was drawn from.
					const uint32_t* data = reinterpret_cast<const uint32_t*>(bytes.data());
					const size_t count = bytes.size() / (2 * sizeof(uint32_t));
					const uint32_t epoch16 = epoch & 0xFFFF;

					Streaming::Requests requests;
					requests.epoch = epoch;
					for (size_t id = 0; id < count; id++)
					{
						const uint32_t want = data[id * 2];
						const uint32_t touched = data[id * 2 + 1];
						if (touched != 0)
							requests.touched.push_back({ static_cast<uint32_t>(id), touched });
						// The half-float error bits order like their values for positive halves.
						if (want != 0 && (want >> 16) == epoch16)
							requests.wanted.push_back({ static_cast<uint32_t>(id), want & 0xFFFF });
					}
					Residency::get().streamer->submit(std::move(requests));
				});
		}
	}
}
