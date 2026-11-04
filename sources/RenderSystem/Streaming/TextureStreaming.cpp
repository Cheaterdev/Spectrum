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
		return std::string_view(HAL::get_backend_name()) == "d3d12";
	}

	void Manager::set_pool_tiles(uint32_t tiles)
	{
		std::lock_guard<std::mutex> g(m);
		pool_tiles = std::max(tiles, 2u);
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

	std::shared_ptr<Streamed> Manager::create(const HAL::texture_data::ptr& data, const std::wstring& name)
	{
		ASSERT(supported());
		ASSERT(data && data->array_size == 1 && data->depth <= 1);

		auto& device = RenderSystem::get().device();

		auto desc = HAL::ResourceDesc::Tex2D(data->format.to_typeless(), { data->width, data->height }, 1, data->mip_maps,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::Virtual | HAL::ResFlags::NoSamplerFeedback);

		auto streamed = std::make_shared<Streamed>();
		streamed->texture = std::make_shared<HAL::Texture>(device, desc, HAL::TextureLayout::SHADER_RESOURCE);
		streamed->texture->resource->set_name(convert(name));
		streamed->source = data;
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

		std::lock_guard<std::mutex> g(m);

		streamed->base_id = allocate_ids(pages);
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

		// The mips below a tile are mapped and filled once, whole.
		if (tiled.packed_mip_count)
		{
			Op op;
			op.kind = Op::Kind::Packed;
			op.owner = streamed.get();
			op.texture = streamed->texture;
			for (uint32_t mip = standard; mip < data->mip_maps; mip++)
			{
				const auto& source = *data->array[0]->mips[mip];
				Copy copy;
				copy.subresource = mip;
				copy.box = ivec3(int(source.width), int(source.height), 1);
				copy.row_stride = source.width_stride;
				copy.data = source.data;
				op.copies.push_back(std::move(copy));
			}
			ops.push_back(std::move(op));
		}

		texture_count++;

		if (pages)
		{
			streamer->add(*streamed);

			uint32_t roots = 0;
			for (uint32_t p = 0; p < pages; p++)
				roots += streamed->is_root(p);

			streamer->set_budget(pool_tiles);
			streamer->ensure_pool(pool_tiles);
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

	// The texels of one tile, as the rows of the box the copy writes (block rows for a compressed format).
	void Manager::cut_tile(const Streamed& streamed, uint32_t mip, uint32_t x, uint32_t y, Copy& out)
	{
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

	Streaming::Backend::Load Manager::begin_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file)
	{
		Streamed& streamed = static_cast<Streamed&>(resource);
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

		for (Op& op : ops)
		{
			PROFILE(L"texture_stream_op");

			auto& resource = op.texture->resource;
			auto& tiled = resource->get_tiled_manager();

			HAL::update_tiling_info info;
			info.resource = resource.get();

			switch (op.kind)
			{
			case Op::Kind::Map:
				tiled.map_tile(info, uint3(op.x, op.y, 0u), tiled.flat_index(0, op.mip), pool[op.slot]);
				break;
			case Op::Kind::Unmap:
				tiled.unmap_tile(info, uint3(op.x, op.y, 0u), tiled.flat_index(0, op.mip));
				break;
			case Op::Kind::Packed:
				tiled.load_packed(info);
				break;
			}

			list->update_tilings(std::move(info));

			for (const Copy& copy : op.copies)
				list->get_copy().update_texture(resource, copy.offset, copy.box, copy.subresource,
					reinterpret_cast<const char*>(copy.data.data()), copy.row_stride);

			touched.insert(op.owner);
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

		for (const InfoWrite& write : info_writes)
		{
			uint value = write.value;
			list->get_copy().update(stream_info, write.view, std::span{ &value, 1 });
		}
		info_writes.clear();
	}
}
