module Graphics:TextureStreaming;

import :TextureStreaming;

import Core;
import HAL;
import :Texture;
import :Streaming;

import RenderSystem;

namespace
{
	VariableContext& texture_streaming_context()
	{
		static auto ctx = VariableContext::create(L"Texture streaming");
		return *ctx;
	}

	Variable<int> g_mip_limit = { 0, "Mip limit (finest allowed)", &texture_streaming_context(), 0, 12 };
	Variable<TextureStreaming::DebugView> g_debug_view = { TextureStreaming::DebugView::Off, "Debug view", &texture_streaming_context() };

	// Textures made from now on keep every mip up to this size resident, so ray tracing never sees less than that.
	Variable<int> g_resident_size = { 512, "Always-resident size (px)", &texture_streaming_context(), 0, 4096 };

	// Most video memory the streamed tiles may take, in 64 KB tiles, on top of the always-resident ones (the roots, which
	// are never evicted). Heap memory is taken only as tiles load; lowering it evicts tiles down to the new budget and
	// gives the freed heap memory back.
	Variable<int> g_pool_tiles = { 1024, "Pool size (64 KB tiles)", &texture_streaming_context(), 64, 65536 };
}

namespace TextureStreaming
{
	int Manager::mip_limit()
	{
		return g_mip_limit;
	}

	DebugView Manager::debug_view()
	{
		return g_debug_view;
	}

	int Manager::resident_size()
	{
		return g_resident_size;
	}

	void Manager::set_resident_size(int texels)
	{
		g_resident_size = texels;
	}

	uint32_t Streamed::shader_mip() const
	{
		if (mips.empty())
			return 0;

		return std::min(std::max(min_mip(), static_cast<uint32_t>(std::max(Manager::mip_limit(), 0))), static_cast<uint32_t>(mips.size()) - 1);
	}

	void Streamed::locate(uint32_t page, uint32_t& mip, uint32_t& x, uint32_t& y) const
	{
		mip = static_cast<uint32_t>(mips.size()) - 1;
		while (mip > 0 && mips[mip].first_page > page)
			mip--;

		const uint32_t in_mip = page - mips[mip].first_page;
		x = in_mip % mips[mip].tiles_x;
		y = in_mip / mips[mip].tiles_x;
	}

	uint32_t Streamed::complete_mip() const
	{
		if (mips.empty())
			return 0;

		// The root mip is always there; each finer mip counts once all its tiles are shown.
		uint32_t mip = static_cast<uint32_t>(mips.size()) - 1;
		while (mip > 0 && mips[mip - 1].resident == tiles_in_mip(mip - 1))
			mip--;
		return mip;
	}

	Streamed::~Streamed()
	{
		if (Manager::is_good())
			Manager::get().release(*this);
	}

	Manager::Manager()
		: streamer(std::make_shared<Streaming::Streamer>(static_cast<Streaming::Backend&>(*this), MaxPageIds))
		, m(streamer->mutex())
		, stream_info(RenderSystem::get().device(), ViewCapacity, HAL::counterType::NONE, HAL::ResFlags::ShaderResource)
		, wanted(RenderSystem::get().device(), ViewCapacity, HAL::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)
	{
		// Heap memory is taken as tiles load and given back as they go, up to the pool size.
		streamer->use_demand_pool();

		// Every shader may read it from the first draw on, so it holds defined values before anything is recorded.
		auto list = RenderSystem::get().device().get_upload_list();
		std::vector<uint> zeros(ViewCapacity, 0);
		list->get_copy().update(stream_info, 0, std::span<uint>(zeros));
		list->get_copy().update(wanted, 0, std::span<uint>(zeros));
		list->execute_and_wait();
	}

	void Manager::bind_view(const HAL::Resource* resource, uint32_t view_index)
	{
		std::lock_guard<std::mutex> g(m);

		auto it = by_resource.find(resource);
		if (it == by_resource.end())
			return;

		Streamed& streamed = *it->second;
		if (std::find(streamed.views.begin(), streamed.views.end(), view_index) == streamed.views.end())
			streamed.views.push_back(view_index);

		info_writes.push_back({ view_index, streamed.shader_mip() + 1 });
	}

	Manager::~Manager()
	{
		// Before any member goes: the streamer thread calls back into this object.
		streamer->stop();

		for (auto& tile : pool)
			tile.handle.Free();
	}

	bool Manager::supported()
	{
		return RenderSystem::is_good() && RenderSystem::get().device().get_properties().streamed_textures;
	}

	void Manager::set_pool_tiles(uint32_t tiles)
	{
		g_pool_tiles = static_cast<int>(std::max(tiles, 2u));
		apply_pool_size();
	}

	// Makes the pool the Variable's size: what the streamer budgets and, when it grew, maps.
	void Manager::apply_pool_size()
	{
		std::lock_guard<std::mutex> g(m);

		const uint32_t wanted = static_cast<uint32_t>(std::max<int>(g_pool_tiles, 2));
		if (wanted == pool_tiles)
			return;

		pool_tiles = wanted;
		if (texture_count)
		{
			streamer->set_budget(pool_tiles);
			streamer->trim_pool();
		}
	}

	void Manager::release_slot(uint32_t slot)
	{
		pool[slot].handle.Free();
		pool[slot] = HAL::TileHeapPosition{};
	}

	void Manager::restore_slot(uint32_t slot)
	{
		pool[slot] = RenderSystem::get().device().get_static_gpu_data().create_tile(HAL::HeapType::DEFAULT);
	}

	uint32_t Manager::allocate_ids(uint32_t count)
	{
		for (size_t i = 0; i < free_ids.size(); i++)
		{
			if (free_ids[i].count < count)
				continue;

			const uint32_t first = free_ids[i].first;
			free_ids[i].first += count;
			free_ids[i].count -= count;
			if (free_ids[i].count == 0)
				free_ids.erase(free_ids.begin() + i);
			return first;
		}

		ASSERT(next_id + count <= MaxPageIds);
		const uint32_t first = next_id;
		next_id += count;
		return first;
	}

	void Manager::free_id_range(uint32_t first, uint32_t count)
	{
		if (count == 0)
			return;

		if (first + count == next_id)
		{
			next_id = first;
			return;
		}

		free_ids.push_back({ first, count });
	}

	void Manager::allocate_pool(uint32_t count)
	{
		auto& storage = RenderSystem::get().device().get_static_gpu_data();
		for (uint32_t i = 0; i < count; i++)
			pool.push_back(storage.create_tile(HAL::HeapType::DEFAULT));
	}

	bool TileFile::read(uint64_t offset, void* destination, size_t size)
	{
		std::lock_guard<std::mutex> g(file_mutex);
		return read_unlocked(offset, destination, size);
	}

	bool TileFile::read_tile(uint32_t mip, uint32_t x, uint32_t y, void* destination)
	{
		std::lock_guard<std::mutex> g(file_mutex);
		return read_unlocked(mips[mip].offset + (uint64_t(y) * mips[mip].tiles_x + x) * TileBytes, destination, TileBytes);
	}

	void TileFile::locate_tile(uint32_t mip, uint32_t x, uint32_t y, std::filesystem::path& file, uint64_t& offset)
	{
		std::lock_guard<std::mutex> g(file_mutex);
		file = path;
		offset = mips[mip].offset + (uint64_t(y) * mips[mip].tiles_x + x) * TileBytes;
	}

	void TileFile::relocate(const TileFile& fresh)
	{
		std::lock_guard<std::mutex> g(file_mutex);
		if (RenderSystem::is_good() && !path.empty())
			RenderSystem::get().device().get_ds_queue().close_file(path);
		mips = fresh.mips;
		packed = fresh.packed;
		path = fresh.path;
		stream.close();
	}

	TileFile::~TileFile()
	{
		// DirectStorage keeps the files it reads open: the asset file may be deleted or written over once nothing reads it.
		if (RenderSystem::is_good() && !path.empty())
			RenderSystem::get().device().get_ds_queue().close_file(path);
	}

	bool TileFile::read_all()
	{
		std::lock_guard<std::mutex> g(file_mutex);

		tile_bytes.resize(mips.size());
		for (size_t i = 0; i < mips.size(); i++)
		{
			tile_bytes[i].resize(uint64_t(mips[i].tiles_x) * mips[i].tiles_y * TileBytes);
			if (!read_unlocked(mips[i].offset, tile_bytes[i].data(), tile_bytes[i].size()))
				return false;
		}

		packed_bytes.resize(packed.size());
		for (size_t i = 0; i < packed.size(); i++)
		{
			packed_bytes[i].resize(packed[i].size);
			if (!read_unlocked(packed[i].offset, packed_bytes[i].data(), packed_bytes[i].size()))
				return false;
		}
		return true;
	}

	bool TileFile::read_unlocked(uint64_t offset, void* destination, size_t size)
	{
		if (!stream.is_open())
			stream.open(path, std::ios::binary | std::ios::in);
		if (!stream.is_open())
			return false;

		stream.clear();
		stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
		stream.read(static_cast<char*>(destination), static_cast<std::streamsize>(size));
		return static_cast<size_t>(stream.gcount()) == size;
	}

	std::shared_ptr<Streamed> Manager::create(const HAL::texture_data::ptr& data, const std::wstring& name)
	{
		ASSERT(data);
		return create_streamed(*data, data, nullptr, name);
	}

	std::shared_ptr<Streamed> Manager::create(const std::shared_ptr<TileFile>& file, const std::wstring& name)
	{
		ASSERT(file);
		return create_streamed(*file, nullptr, file, name);
	}

	std::shared_ptr<TileFile> Manager::bake(const HAL::texture_data::ptr& data)
	{
		ASSERT(supported());
		ASSERT(data && data->array_size == 1 && data->depth <= 1);

		auto& device = RenderSystem::get().device();

		auto desc = HAL::ResourceDesc::Tex2D(data->format.to_typeless(), { data->width, data->height }, 1, data->mip_maps,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::Virtual | HAL::ResFlags::NoSamplerFeedback | HAL::ResFlags::RestInCommon);
		auto texture = std::make_shared<HAL::Texture>(device, desc, HAL::TextureLayout::COPY_QUEUE);
		auto& tiled = texture->resource->get_tiled_manager();
		const uint32_t standard = tiled.unpacked_mip_count;

		auto file = std::make_shared<TileFile>();
		static_cast<HAL::texture_data_header&>(*file) = *data;

		// Every tile of every standard mip gets memory and the texels of its mip.
		{
			auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			list->begin(L"TextureStreaming bake fill");

			for (uint32_t mip = 0; mip < standard; mip++)
			{
				const uint3 count = tiled.get_tiles_count(0, mip);
				file->mips.push_back({ count.x, count.y, 0 });
				tiled.load_tiles(list.get(), uint3(0, 0, 0), uint3(count.x - 1, count.y - 1, 0), 0, mip);

				const auto& source = *data->array[0]->mips[mip];
				list->get_copy().update_texture(texture->resource, ivec3(0, 0, 0), ivec3(int(source.width), int(source.height), 1), mip,
					reinterpret_cast<const char*>(source.data.data()), source.width_stride);
			}
			list->execute_and_wait();
		}

		// Then the tiles come back as the GPU lays them out.
		file->tile_bytes.resize(standard);
		{
			auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			list->begin(L"TextureStreaming bake read");

			std::vector<std::future<bool>> pending;
			for (uint32_t mip = 0; mip < standard; mip++)
			{
				auto& bytes = file->tile_bytes[mip];
				pending.push_back(list->get_copy().read_tiles(texture->resource.get(), uint3(0, 0, 0), uint3(file->mips[mip].tiles_x, file->mips[mip].tiles_y, 1), mip,
					[&bytes](std::span<std::byte> memory) { bytes.assign(memory.begin(), memory.end()); }));
			}
			list->execute_and_wait();
			for (auto& f : pending)
				f.wait();
		}

		// The mips below a tile are plain rows: the texture is mapped and filled with them whole.
		for (uint32_t mip = standard; mip < data->mip_maps; mip++)
		{
			const auto& source = *data->array[0]->mips[mip];
			file->packed.push_back({ source.width, source.height, source.width_stride, source.data.size(), 0 });
			file->packed_bytes.push_back(source.data);
		}

		return file;
	}

	std::shared_ptr<Streamed> Manager::create_streamed(const HAL::texture_data_header& header, const HAL::texture_data::ptr& data, const std::shared_ptr<TileFile>& file, const std::wstring& name)
	{
		ASSERT(supported());
		ASSERT(header.array_size == 1 && header.depth <= 1);

		auto& device = RenderSystem::get().device();

		auto desc = HAL::ResourceDesc::Tex2D(header.format.to_typeless(), { header.width, header.height }, 1, header.mip_maps,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::Virtual | HAL::ResFlags::NoSamplerFeedback | HAL::ResFlags::RestInCommon);

		auto streamed = std::make_shared<Streamed>();
		// COPY_QUEUE is the COMMON layout: shaders read it there, and it is the only layout DirectStorage may write a tile in.
		streamed->texture = std::make_shared<HAL::Texture>(device, desc, HAL::TextureLayout::COPY_QUEUE);
		streamed->texture->resource->set_name(convert(name));
		streamed->source = data;
		streamed->file = file;
		streamed->streamed = true;

		auto& tiled = streamed->texture->resource->get_tiled_manager();
		ASSERT(tiled.is_tiled());

		const uint32_t standard = tiled.unpacked_mip_count;

		uint32_t pages = 0;
		for (uint32_t mip = 0; mip < standard; mip++)
		{
			const uint3 count = tiled.get_tiles_count(0, mip);
			streamed->mips.push_back({ pages, count.x, count.y, 0 });
			pages += count.x * count.y;
		}

		// The coarsest standard mip is always a root; so is every finer one that fits the resident size.
		streamed->root_mip = standard ? standard - 1 : 0;
		while (streamed->root_mip > 0)
		{
			const uint32_t finer = std::max(std::max(header.width >> (streamed->root_mip - 1), 1u), std::max(header.height >> (streamed->root_mip - 1), 1u));
			if (finer > static_cast<uint32_t>(std::max(resident_size(), 0)))
				break;
			streamed->root_mip--;
		}

		// Everything that belongs to this texture alone is built before the manager's lock is taken: page states (their
		// parents are texture-local page numbers, the ids are added to base_id later) and the packed mips' copies,
		// which read the asset file.
		{
			PROFILE(L"texture_stream_add_prepare");
			streamed->states.resize(pages);

			// A tile waits for the tile of the next coarser mip that covers it.
			for (uint32_t mip = 0; mip + 1 < standard; mip++)
			{
				const auto& finer = streamed->mips[mip];
				const auto& coarser = streamed->mips[mip + 1];
				for (uint32_t y = 0; y < finer.tiles_y; y++)
					for (uint32_t x = 0; x < finer.tiles_x; x++)
						streamed->states[streamed->page_of(mip, x, y)].parents.push_back(
							streamed->page_of(mip + 1, std::min(x / 2, coarser.tiles_x - 1), std::min(y / 2, coarser.tiles_y - 1)));
			}
		}

		// The mips below a tile are mapped and filled once, whole.
		std::optional<Op> packed_op;
		if (tiled.packed_mip_count)
		{
			PROFILE(L"texture_stream_add_read_packed");
			Op op;
			op.kind = Op::Kind::Packed;
			op.owner = streamed.get();
			op.texture = streamed->texture;
			for (uint32_t mip = standard; mip < header.mip_maps; mip++)
			{
				Copy copy;
				copy.subresource = mip;

				if (data)
				{
					const auto& source = *data->array[0]->mips[mip];
					copy.box = ivec3(int(source.width), int(source.height), 1);
					copy.row_stride = source.width_stride;
					copy.data = source.data;
				}
				else
				{
					const auto& packed = file->packed[mip - standard];
					copy.box = ivec3(int(packed.width), int(packed.height), 1);
					copy.row_stride = packed.row_stride;
					copy.data.resize(packed.size);
					if (!file->read(packed.offset, copy.data.data(), copy.data.size()))
						throw std::runtime_error("texture asset file is shorter than its tile index says");
				}
				op.copies.push_back(std::move(copy));
			}
			packed_op = std::move(op);
		}

		std::unique_lock<std::mutex> g(m, std::defer_lock);
		{
			PROFILE(L"texture_stream_add_lock_wait");
			g.lock();
		}
		PROFILE(L"texture_stream_add_locked");

		streamed->base_id = allocate_ids(pages);
		if (packed_op)
			ops.push_back(std::move(*packed_op));

		texture_count++;

		if (pages)
		{
			PROFILE(L"texture_stream_add_roots");
			streamer->add(*streamed);

			uint32_t roots = 0;
			for (uint32_t p = 0; p < pages; p++)
				roots += streamed->is_root(p);

			pool_tiles = static_cast<uint32_t>(std::max<int>(g_pool_tiles, 2));
			streamer->set_budget(pool_tiles);
			streamer->ensure_free(roots);

			for (uint32_t p = 0; p < pages; p++)
				if (streamed->is_root(p))
					streamer->load_root(*streamed, p);

			streamer->start();
		}

		streamed->published_mip = streamed->complete_mip();
		by_resource[streamed->texture->resource.get()] = streamed.get();
		return streamed;
	}

	void Manager::release(Streamed& streamed)
	{
		std::lock_guard<std::mutex> g(m);

		streamer->remove(streamed);

		// The descriptors may be reused by another texture: until then nothing is clamped through them.
		by_resource.erase(streamed.texture->resource.get());
		for (uint32_t view : streamed.views)
			info_writes.push_back({ view, 0 });

		const uint32_t count = streamed.page_count();
		for (uint32_t p = 0; p < count; p++)
			staged.erase(streamed.base_id + p);

		ops.erase(std::remove_if(ops.begin(), ops.end(), [&](const Op& op) { return op.owner == &streamed; }), ops.end());
		free_id_range(streamed.base_id, count);
		texture_count--;
	}

	// The texels of one tile, as the rows of the box the copy writes (block rows for a compressed format). Only for a
	// texture with its texels in memory: a file texture's tiles are read by start_loads.
	void Manager::cut_tile(const Streamed& streamed, uint32_t mip, uint32_t x, uint32_t y, Copy& out)
	{
		ASSERT(!streamed.file);

		const auto& source = *streamed.source->array[0]->mips[mip];
		const uint3 shape = streamed.texture->resource->get_tiled_manager().get_tile_shape();

		const uint32_t block = streamed.source->format.is_compressed() ? 4 : 1;
		const uint32_t x0 = x * shape.x;
		const uint32_t y0 = y * shape.y;
		const uint32_t width = std::min(shape.x, source.width - x0);
		const uint32_t height = std::min(shape.y, source.height - y0);

		const uint32_t blocks_in_row = (source.width + block - 1) / block;
		const uint32_t block_bytes = source.width_stride / blocks_in_row;
		const uint32_t tile_blocks_x = (width + block - 1) / block;
		const uint32_t tile_rows = (height + block - 1) / block;
		const uint32_t row_bytes = tile_blocks_x * block_bytes;

		out.subresource = mip;
		out.offset = ivec3(int(x0), int(y0), 0);
		out.box = ivec3(int(width), int(height), 1);
		out.row_stride = row_bytes;
		out.data.resize(size_t(row_bytes) * tile_rows);

		for (uint32_t r = 0; r < tile_rows; r++)
			std::memcpy(out.data.data() + size_t(r) * row_bytes,
				source.data.data() + size_t(y0 / block + r) * source.width_stride + size_t(x0 / block) * block_bytes, row_bytes);
	}

	Manager::Op Manager::make_map(Streamed& streamed, uint32_t page, uint32_t slot)
	{
		Op op;
		op.kind = Op::Kind::Map;
		op.owner = &streamed;
		op.texture = streamed.texture;
		op.slot = slot;
		streamed.locate(page, op.mip, op.x, op.y);
		op.copies.emplace_back();
		cut_tile(streamed, op.mip, op.x, op.y, op.copies.back());
		return op;
	}

	// A tile of an asset file goes to the GPU without passing through the CPU: the tile is mapped to its slot first (on the
	// copy queue, which is idle, so it is done within a poll or two), and once that has executed DirectStorage reads the
	// file range into it. DirectStorage cannot write an unmapped tile, so the read cannot start with the map. The mapping
	// is recorded here, under the lock (map_tile keeps the tiled manager's own state); start_loads executes it.
	Streaming::Backend::LoadPlan Manager::plan_direct_load(Streamed& streamed, uint32_t page, uint32_t slot)
	{
		Op op;
		op.kind = Op::Kind::Map;
		op.owner = &streamed;
		op.texture = streamed.texture;
		op.slot = slot;
		op.premapped = true;
		streamed.locate(page, op.mip, op.x, op.y);

		// An unmap of this tile still waiting for the frame list would undo the map made here.
		ops.erase(std::remove_if(ops.begin(), ops.end(), [&](const Op& other)
			{ return other.kind == Op::Kind::Unmap && other.owner == &streamed && other.mip == op.mip && other.x == op.x && other.y == op.y; }), ops.end());

		auto data = std::make_shared<TilePlan>();
		data->direct = true;

		HAL::StorageRequest& request = data->request;
		request.resource = streamed.texture->resource;
		streamed.file->locate_tile(op.mip, op.x, op.y, request.file, request.file_offset);
		request.size = TileBytes;
		request.uncompressed_size = TileBytes;
		request.compressed = false;
		request.operation = HAL::StorageRequest::Tiles{ op.mip, uint3(op.x, op.y, 0u), uint3(1u, 1u, 1u) };

		auto& tiled = request.resource->get_tiled_manager();
		data->map.resource = request.resource.get();
		tiled.map_tile(data->map, uint3(op.x, op.y, 0u), tiled.flat_index(0, op.mip), pool[slot]);

		staged[static_cast<uint32_t>(streamed.base_id + page)] = std::move(op);

		Backend::LoadPlan plan;
		plan.from_file = true;
		plan.backend_data = std::move(data);
		return plan;
	}

	bool Manager::TileBatch::poll()
	{
		if (!requested)
		{
			if (!mapped.is_completed())
				return false;

			done = RenderSystem::get().device().get_ds_queue().execute(std::span<const HAL::StorageRequest>(requests));
			requested = true;
		}
		return done.is_completed();
	}

	Manager::TileBatch::~TileBatch()
	{
		// The textures must outlive the mapping and the reads.
		if (requested)
			done.wait();
		else
			mapped.wait();
	}

	Streaming::Backend::LoadPlan Manager::plan_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file)
	{
		Streamed& streamed = static_cast<Streamed&>(resource);

		if (allow_file && streamed.file)
			return plan_direct_load(streamed, page, slot);

		if (streamed.file)
		{
			// A root of a file texture: the tile is already in the GPU's layout, one range of the file, read by start_loads.
			// offset holds the tile's coordinates.
			auto data = std::make_shared<TilePlan>();
			data->file = streamed.file;

			Op& op = data->op;
			op.kind = Op::Kind::Map;
			op.owner = &streamed;
			op.texture = streamed.texture;
			op.slot = slot;
			streamed.locate(page, op.mip, op.x, op.y);

			Copy& copy = op.copies.emplace_back();
			copy.subresource = op.mip;
			copy.offset = ivec3(int(op.x), int(op.y), 0);
			copy.box = ivec3(1, 1, 1);
			copy.tiles = true;
			copy.data.resize(TileBytes);

			Backend::LoadPlan plan;
			plan.backend_data = std::move(data);
			return plan;
		}

		Op op = make_map(streamed, page, slot);

		if (allow_file)
		{
			// Visible only once the streamer shows it.
			staged[static_cast<uint32_t>(streamed.base_id + page)] = std::move(op);
		}
		else
		{
			// A root: shown at once.
			streamed.mips[op.mip].resident++;
			ops.push_back(std::move(op));
		}
		return {};
	}

	void Manager::start_loads(std::span<Streaming::Backend::LoadPlan> plans)
	{
		std::shared_ptr<TileBatch> batch;
		HAL::CommandList::ptr list;

		for (auto& plan : plans)
		{
			auto* data = static_cast<TilePlan*>(plan.backend_data.get());
			if (!data)
				continue;

			if (data->direct)
			{
				if (!batch)
				{
					batch = std::make_shared<TileBatch>();
					list = RenderSystem::get().device().get_queue(HAL::CommandListType::COPY)->get_free_list();
					list->begin(L"TextureStreaming tile map");
				}
				list->update_tilings(std::move(data->map));
				batch->requests.push_back(std::move(data->request));
				plan.poll = [batch]() { return batch->poll(); };
				plan.backend_data.reset();   // the batch holds what the map and the read need now
				continue;
			}

			PROFILE(L"texture_stream_read_root");
			const Op& op = data->op;
			if (!data->file->read_tile(op.mip, op.x, op.y, data->op.copies.back().data.data()))
				throw std::runtime_error("texture asset file is shorter than its tile index says");
		}

		if (batch)
		{
			PROFILE(L"texture_stream_map_batch");
			batch->mapped = list->execute();
		}
	}

	void Manager::finish_loads(std::span<Streaming::Backend::LoadPlan> plans)
	{
		for (auto& plan : plans)
		{
			auto* data = static_cast<TilePlan*>(plan.backend_data.get());
			if (!data || data->direct || !data->file)
				continue;

			// A root: shown at once.
			data->op.owner->mips[data->op.mip].resident++;
			ops.push_back(std::move(data->op));
			plan.backend_data.reset();
		}
	}

	void Manager::show(Streaming::Resource& resource, uint32_t page, uint32_t slot)
	{
		Streamed& streamed = static_cast<Streamed&>(resource);

		auto it = staged.find(static_cast<uint32_t>(streamed.base_id + page));
		ASSERT(it != staged.end() && it->second.slot == slot);

		streamed.mips[it->second.mip].resident++;
		ops.push_back(std::move(it->second));
		staged.erase(it);
	}

	void Manager::hide(Streaming::Resource& resource, uint32_t page)
	{
		Streamed& streamed = static_cast<Streamed&>(resource);

		Op op;
		op.kind = Op::Kind::Unmap;
		op.owner = &streamed;
		op.texture = streamed.texture;
		streamed.locate(page, op.mip, op.x, op.y);
		streamed.mips[op.mip].resident--;
		ops.push_back(std::move(op));
	}

	void Manager::set_flags(Streaming::Resource&, uint32_t, uint32_t, uint32_t)
	{
		// The GPU does not compact this client's feedback: the tile flags have no reader.
	}

	void Manager::request(std::span<const Demand> demands)
	{
		Streaming::Requests requests;

		std::lock_guard<std::mutex> g(m);
		requests.epoch = streamer->epoch();

		for (const Demand& demand : demands)
		{
			const Streamed& streamed = *demand.texture;
			for (uint32_t mip = 0; mip < streamed.standard_mips(); mip++)
				for (uint32_t i = 0; i < streamed.tiles_in_mip(mip); i++)
				{
					const uint32_t page = streamed.mips[mip].first_page + i;
					const auto& state = streamed.states[page];

					if (mip >= demand.mip)
					{
						if (state.status == Streaming::PageState::Status::Missing)
							requests.wanted.push_back({ streamed.base_id + page, mip });
					}
					else if (state.status == Streaming::PageState::Status::Resident && !streamed.is_root(page))
					{
						requests.victims.push_back({ streamed.base_id + page, state.last_touched });
					}
				}
		}

		streamer->submit(std::move(requests));
	}

	void Manager::collect_feedback(HAL::CommandList::ptr& list)
	{
		PROFILE(L"texture_stream_collect");

		uint64_t frame;
		{
			std::lock_guard<std::mutex> g(m);
			frame = ++feedback_frame;
		}

		list->get_copy().read_buffer(wanted.resource.get(), 0, uint64_t(ViewCapacity) * sizeof(uint32_t),
			[frame](std::span<std::byte> bytes)
			{
				if (!Manager::is_good() || bytes.size() < ViewCapacity * sizeof(uint32_t))
					return;

				const uint32_t* data = reinterpret_cast<const uint32_t*>(bytes.data());

				Manager& manager = Manager::get();
				std::lock_guard<std::mutex> g(manager.m);
				for (auto& [resource, streamed] : manager.by_resource)
				{
					uint32_t value = 0;
					for (uint32_t view : streamed->views)
						value = std::max(value, data[view]);

					if (value)
					{
						streamed->wanted_mip = 16u - std::min(value, 16u);
						streamed->last_seen = frame;
					}
				}
			});

		// Cleared after the copy above in this list, so the draws that follow report into a fresh buffer.
		list->clear_uav(wanted.rwRAW, vec4(0, 0, 0, 0));
	}

	void Manager::request_from_feedback()
	{
		PROFILE(L"texture_stream_request");

		apply_pool_size();

		std::vector<Demand> demands;
		{
			std::lock_guard<std::mutex> g(m);
			for (auto& [resource, streamed] : by_resource)
			{
				const uint32_t coarsest = streamed->standard_mips() - 1;
				const bool seen = streamed->last_seen && feedback_frame - streamed->last_seen < HoldFrames;
				const uint32_t wanted_mip = std::max(streamed->wanted_mip, static_cast<uint32_t>(std::max(mip_limit(), 0)));
				demands.push_back({ streamed, seen ? std::min(wanted_mip, coarsest) : coarsest });
			}
		}

		if (!demands.empty())
			request(demands);
	}

	Manager::Stats Manager::stats()
	{
		return streamer->stats();
	}

	void Manager::prepare(HAL::CommandList::ptr& list)
	{
		std::lock_guard<std::mutex> g(m);

		streamer->advance_epoch();
		streamer->prepare(list);

		std::unordered_set<Streamed*> touched;

		// One update_tilings per texture: all its maps and unmaps of this frame go into one info (its tiles are keyed by
		// heap, so several fit), the changes of a tile that appears more than once collapse into the last one, and
		// then the copies of the texture follow. Textures keep the order of their first op.
		{
			PROFILE(L"texture_stream_ops");

			struct TextureOps
			{
				HAL::Resource* resource;
				std::vector<size_t> indices;   // into `ops`, in order
			};
			std::vector<TextureOps> textures;
			std::unordered_map<const HAL::Resource*, size_t> texture_of;
			for (size_t i = 0; i < ops.size(); i++)
			{
				HAL::Resource* resource = ops[i].texture->resource.get();
				auto [it, added] = texture_of.try_emplace(resource, textures.size());
				if (added)
					textures.push_back({ resource, {} });
				textures[it->second].indices.push_back(i);
			}

			const auto changes_tile = [](const Op& op) { return op.kind == Op::Kind::Unmap || (op.kind == Op::Kind::Map && !op.premapped); };
			const auto tile_key = [](const Op& op) { return (uint64_t(op.mip) << 48) | (uint64_t(op.y) << 24) | op.x; };

			for (const TextureOps& texture : textures)
			{
				PROFILE(L"texture_stream_texture_ops");

				auto& tiled = texture.resource->get_tiled_manager();

				std::unordered_map<uint64_t, size_t> last;   // tile -> the op that decides its final state
				for (size_t i : texture.indices)
					if (changes_tile(ops[i]))
						last[tile_key(ops[i])] = i;

				HAL::update_tiling_info info;
				info.resource = texture.resource;
				bool recorded = false;

				for (size_t i : texture.indices)
				{
					const Op& op = ops[i];
					if (op.kind == Op::Kind::Packed)
					{
						tiled.load_packed(info);
						recorded = true;
					}
					else if (changes_tile(op) && last[tile_key(op)] == i)
					{
						if (op.kind == Op::Kind::Map)
							tiled.map_tile(info, uint3(op.x, op.y, 0u), tiled.flat_index(0, op.mip), pool[op.slot]);
						else
							tiled.unmap_tile(info, uint3(op.x, op.y, 0u), tiled.flat_index(0, op.mip));
						recorded = true;
					}
				}

				if (recorded)
					list->update_tilings(std::move(info));

				for (size_t i : texture.indices)
				{
					const Op& op = ops[i];
					touched.insert(op.owner);

					// A map that a later op of the same tile replaced: its tile never holds this data, and may not be mapped at all.
					if (changes_tile(op) && last[tile_key(op)] != i)
						continue;
					if (op.kind == Op::Kind::Map && op.premapped)
						continue;

					for (const Copy& copy : op.copies)
					{
						auto& resource = op.texture->resource;
						if (copy.tiles)
							list->get_copy().write_tiles(resource.get(), uint3(uint32_t(copy.offset.x), uint32_t(copy.offset.y), 0), uint3(1, 1, 1), copy.subresource,
								std::span<const std::byte>(reinterpret_cast<const std::byte*>(copy.data.data()), copy.data.size()));
						else
							list->get_copy().update_texture(resource, copy.offset, copy.box, copy.subresource,
								reinterpret_cast<const char*>(copy.data.data()), copy.row_stride);
					}
				}
			}
		}
		ops.clear();

		// The changes are in this list: draws recorded after it may use what they made resident.
		for (Streamed* streamed : touched)
		{
			streamed->published_mip = streamed->complete_mip();
			for (uint32_t view : streamed->views)
				info_writes.push_back({ view, streamed->shader_mip() + 1 });
		}

		// A new limit applies to every streamed texture at once, not as their tiles change.
		if (applied_limit != mip_limit())
		{
			applied_limit = mip_limit();
			for (auto& [resource, streamed] : by_resource)
				if (!touched.count(streamed))
					for (uint32_t view : streamed->views)
						info_writes.push_back({ view, streamed->shader_mip() + 1 });
		}

		if (!info_writes.empty())
		{
			PROFILE(L"texture_stream_info_writes");

			// A view written more than once keeps its last value.
			std::stable_sort(info_writes.begin(), info_writes.end(), [](const InfoWrite& a, const InfoWrite& b) { return a.view < b.view; });
			size_t kept = 0;
			for (size_t i = 0; i < info_writes.size(); i++)
				if (i + 1 == info_writes.size() || info_writes[i + 1].view != info_writes[i].view)
					info_writes[kept++] = info_writes[i];
			info_writes.resize(kept);

			std::vector<HAL::CopyContext::BufferWrite> writes;
			writes.reserve(info_writes.size());
			for (const InfoWrite& write : info_writes)
				writes.push_back({ stream_info.desc.offset + uint64_t(write.view) * sizeof(uint32_t), std::as_bytes(std::span<const uint32_t>(&write.value, 1)) });
			list->get_copy().update_buffer_batch(stream_info.resource.get(), writes);
			info_writes.clear();
		}
	}
}
