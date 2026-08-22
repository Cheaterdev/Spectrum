module HAL:TiledMemoryManager;

import Core;
import HAL;

namespace HAL {

	void TiledResourceManager::map_buffer_part(update_tiling_info& target, size_t offset, size_t size)
	{
		size_t begin = Math::AlignDown(offset, 64 * 1024) / (64 * 1024);
		size_t end = Math::roundUp(offset + size, 64 * 1024) / (64 * 1024) - 1;

		load_tiles_internal(target, { begin,0,0 }, { end,0,0 }, 0, false);
	}

	void TiledResourceManager::load_tile(update_tiling_info& target, uint3 pos, uint storage_index, bool recursive)
	{
		auto& alloc_info = (resource)->alloc_info;
		auto& tile = tiles[storage_index][pos];

		if (!tile.heap_position.heap)
		{
			tile.heap_position = resource->get_device().get_static_gpu_data().create_tile(tile_heap_type);
			mark_tile_mapped(storage_index);
			target.add_tile(tile);
			on_load(ivec4(pos, storage_index));
			if (recursive)
			{
				// Phase 5.9: tiles[]/storage_index are flat-indexed as
				// (mip + slice*unpacked_mip_count) once an array is
				// involved -- "storage_index+1" only means "next coarser mip
				// of THIS SAME slice" while still inside that slice's mip
				// range; must not walk into the next slice's mip 0.
				// Array-aware, but load_packed() itself is still only
				// correct for a single slice (see its own comment) -- an
				// array resource simply never reaches this branch in
				// practice, since callers clamp their usable mip count to
				// unpacked_mip_count.
				bool at_last_standard_mip_of_slice = (unpacked_mip_count == 0) || ((storage_index % unpacked_mip_count) == unpacked_mip_count - 1);
				if (!at_last_standard_mip_of_slice)
				{
					load_tile(target, uint3(pos.x / 2, pos.y / 2, pos.z / 2), storage_index + 1, recursive);
				}
				else
				{
					load_packed(target);
				}
			}
		}
	}

	void  TiledResourceManager::zero_tile(update_tiling_info& target, uint3 pos, uint storage_index)
	{
		auto& tile = tiles[storage_index][pos];

		if (tile.heap_position.heap)
		{
			// Phase 5.9 fix: don't Free() the handle here -- the underlying
			// allocator (PagePool::free()) makes the sub-allocation reusable
			// for a completely unrelated request immediately, with no GPU
			// fence delay, but the GPU hasn't even been told about this
			// UpdateTileMappings unmap yet (that happens later, when this
			// command list's queue submission is processed), let alone
			// finished any in-flight reads of the old content. Keep the
			// handle alive in target.pending_free; it gets actually freed in
			// on_tile_update(), which only runs after a real GPU fence wait
			// confirms this batch has completed.
			target.pending_free.push_back(tile.heap_position.handle);

			tile.heap_position.heap = nullptr;
			mark_tile_unmapped(storage_index);
			target.add_tile(tile);

			on_zero(ivec4(pos, storage_index));


			// Phase 5.9: see the matching comment in load_tile() -- must not
			// walk the parent-mip collapse check across a slice boundary.
			bool at_last_standard_mip_of_slice = (unpacked_mip_count == 0) || ((storage_index % unpacked_mip_count) == unpacked_mip_count - 1);
			if (/*recursive && */!at_last_standard_mip_of_slice)
			{
				uint3 parent_pos = pos / 2;

				uint3 my_pos = parent_pos * 2;

				auto is_mapped = [&](uint3 pos) {

					if (math::all(pos.higher_eq(uint3(0, 0, 0))) && math::all(pos.lower_eq(tiles[storage_index].size())))
					{
						return !!tiles[storage_index][pos].heap_position.heap;
					}

					return false;
				};

				bool can_unmap = true;

				for (int x = 0; x < 2; x++)
					for (int y = 0; y < 2; y++)
						for (int z = 0; z < 2; z++)
							can_unmap = can_unmap && !is_mapped(my_pos + uint3(x, y, z));

				if (can_unmap)
				{
					zero_tile(target, parent_pos, storage_index + 1);
				}

			}
		}
	}

	void TiledResourceManager::load_tiles_internal(update_tiling_info& target, uint3 from, uint3 to, uint storage_index, bool recursive)
	{
		for (uint x = from.x; x <= to.x; x++)
			for (uint y = from.y; y <= to.y; y++)
				for (uint z = from.z; z <= to.z; z++)
				{
					load_tile(target, { x,y,z }, storage_index, recursive);
				}
	}

	void TiledResourceManager::load_tiles(CommandList* list, uint3 from, uint3 to, uint array_slice, uint mip)
	{
		update_tiling_info info;
		info.resource = resource;
		load_tiles_internal(info, from, to, flat_index(array_slice, mip), true);
		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	// One signature for both "just slice 0" (VoxelGI's single-slice usage,
	// via the defaults) and "a specific slice/mip" (VSM's per-page-slot
	// atlas/HiZ) -- see the .ixx declaration comment.
	void TiledResourceManager::zero_tiles(CommandList* list, uint3 from, uint3 to, uint array_slice, uint mip)
	{
		update_tiling_info info;
		info.resource = resource;

		uint storage_index = flat_index(array_slice, mip);
		for (uint x = from.x; x <= to.x; x++)
			for (uint y = from.y; y <= to.y; y++)
				for (uint z = from.z; z <= to.z; z++)
				{
					zero_tile(info, { x,y,z }, storage_index);
				}

		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}


	void TiledResourceManager::load_tiles_batch(CommandList* list, uint3 from, uint3 to, const std::vector<int>& array_slices, uint mip)
	{
		update_tiling_info info;
		info.resource = resource;
		for (int slice : array_slices)
			load_tiles_internal(info, from, to, flat_index((uint)slice, mip), true);

		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::zero_tiles_batch(CommandList* list, uint3 from, uint3 to, const std::vector<int>& array_slices, uint mip)
	{
		update_tiling_info info;
		info.resource = resource;
		for (int slice : array_slices)
		{
			uint storage_index = flat_index((uint)slice, mip);
			for (uint x = from.x; x <= to.x; x++)
				for (uint y = from.y; y <= to.y; y++)
					for (uint z = from.z; z <= to.z; z++)
						zero_tile(info, { x,y,z }, storage_index);
		}

		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::load_tiles(CommandList* list, std::list<uint3>& tiles, uint array_slice, uint mip, bool recursive)
	{
		update_tiling_info info;
		info.resource = resource;

		uint storage_index = flat_index(array_slice, mip);
		for (auto t : tiles)
			load_tile(info, t, storage_index, recursive);


		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}
	void TiledResourceManager::zero_tiles(CommandList* list, std::list<uint3>& tiles_to_remove, uint array_slice, uint mip)
	{
		update_tiling_info info;
		info.resource = resource;

		uint storage_index = flat_index(array_slice, mip);
		for (auto t : tiles_to_remove)
		{
			zero_tile(info, t, storage_index);
		}

		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);

	}



	void TiledResourceManager::zero_tiles(CommandList& list)
	{
		update_tiling_info info;
		info.resource = resource;
		for (int i = 0; i < tiles.size(); i++)
		{
			for (uint x = 0; x < tiles[i].size().x; x++)
				for (uint y = 0; y < tiles[i].size().y; y++)
					for (uint z = 0; z < tiles[i].size().z; z++)
					{
						// Fixed (Phase 5.9 audit): this used to hardcode
						// storage_index=0 regardless of `i` -- every slot
						// beyond the first (any mip>0 for a single-slice
						// resource, or any slice/mip beyond (0,0) for an
						// array) never actually got unmapped by a "zero
						// everything" call, silently leaking its tile
						// mapping. `i` already IS the flat storage index this
						// loop is sized from -- use it.
						zero_tile(info, { x, y, z }, i);
					}
		}

		(list).update_tilings(std::move(info));
	}


	void TiledResourceManager::copy_mappings(CommandList& list, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size)
	{
		update_tiling_info info;

		info.resource = resource;

		info.source = static_cast<Resource*>(source);
		info.source_pos = source_pos;
		info.pos = target_pos;
		info.size = size;
		(list).update_tilings(std::move(info));
	}

	void TiledResourceManager::map_tile(update_tiling_info& info, uint3 pos, TileHeapPosition heap_pos)
	{
		auto& tile = tiles[0][pos];

		if (!tile.heap_position.heap)
			mark_tile_mapped(0);

		tile.heap_position = heap_pos;

		info.add_tile(tile);
	}

	uint3 TiledResourceManager::get_tiles_count(uint array_slice, uint mip)
	{
		return tiles[flat_index(array_slice, mip)].size();
	}
	uint3 TiledResourceManager::get_tile_shape()
	{
		return tile_shape;
	}
	// init_tilings() is implemented in the backend-specific files:
	//   D3D12/HAL.D3D12.TiledMemoryManager.cpp  — real D3D12 implementation
	//   Vulkan/HAL.Vulkan.TiledMemoryManager.cpp — no-op stub

	void TiledResourceManager::load_packed(CommandList& list)
	{
		if (packed_mip_count)
		{
			update_tiling_info info;
			info.resource = resource;
			auto& alloc_info = (resource)->alloc_info;

			if (!packed_tiles.heap_position.heap)
				packed_tiles.heap_position = resource->get_device().get_static_gpu_data().create_tile( HeapType::DEFAULT, packed_mip_count);

			info.add_tile(packed_tiles);

			(list).update_tilings(std::move(info));
		}
	}

	void TiledResourceManager::load_packed(update_tiling_info& info)
	{

		if (packed_mip_count)
		{
			auto& alloc_info = (resource)->alloc_info;

			if (!packed_tiles.heap_position.heap)
				packed_tiles.heap_position = resource->get_device().get_static_gpu_data().create_tile( HeapType::DEFAULT, packed_mip_count);

			info.add_tile(packed_tiles);
		}
	}

	void TiledResourceManager::commit(update_tiling_info& info, CommandList* list)
	{
		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			info.resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::on_tile_update(update_tiling_info& info)
	{
		for (auto& [heap, tiles] : info.tiles)
		{
			for (auto tile : tiles)
			{
				// Fixed (Phase 5.9 audit): this used to index by
				// tile.subresource (the REAL D3D12 subresource), which only
				// coincides with our flat storage_index when the resource
				// has no packed mips and PlaneSlice==0 -- use storage_index,
				// the field that's actually meant to index gpu_tiles[].
				if (tile.storage_index == gpu_tiles.size())
					gpu_packed_tile = tile;
				else
					gpu_tiles[tile.storage_index][tile.pos] = tile;
			}
		}

		// Phase 5.9 fix: safe to actually release the old physical memory now
		// -- this only runs after Queue::execute_internal's gpu_wait_thread has
		// confirmed (via a real fence wait) that the batch which recorded
		// these unmaps has completed on the GPU. See the pending_free member
		// comment in HAL.TiledMemoryManager.ixx.
		for (auto& handle : info.pending_free)
			handle.Free();
		info.pending_free.clear();
	}


	// Implementations moved from HAL.TiledMemoryManager.ixx

	void update_tiling_info::add_tile(ResourceTile tile)
	{
		tiles[tile.heap_position.heap.get()].push_back(tile);
	}

	TiledResourceManager::TiledResourceManager(Resource* resource) : resource(resource) {}

	bool TiledResourceManager::is_tiled() const
	{
		return !tiles.empty();
	}

	bool TiledResourceManager::is_mapped(uint3 pos, uint array_slice, uint mip) const
	{
		return !!tiles[flat_index(array_slice, mip)][pos].heap_position.heap;
	}

	void TiledResourceManager::mark_tile_mapped(uint storage_index)
	{
		if (mapped_tile_count.size() <= storage_index)
		{
			mapped_tile_count.resize(tiles.size(), 0);
			subres_list_pos.resize(tiles.size(), -1);
		}

		if (mapped_tile_count[storage_index]++ == 0)
		{
			subres_list_pos[storage_index] = (int)mapped_subresources.size();
			mapped_subresources.push_back(storage_index);
		}
	}

	void TiledResourceManager::mark_tile_unmapped(uint storage_index)
	{
		// Only ever called right after load_tile/zero_tile/map_tile confirm
		// this exact tile was mapped, so the counter is always >0 here.
		if (--mapped_tile_count[storage_index] == 0)
		{
			// Swap-erase out of the compact list -- position lookup is O(1)
			// via subres_list_pos, so this whole removal is O(1) too.
			int pos      = subres_list_pos[storage_index];
			int last_pos = (int)mapped_subresources.size() - 1;
			UINT moved   = mapped_subresources[last_pos];

			mapped_subresources[pos] = moved;
			subres_list_pos[moved]   = pos;
			mapped_subresources.pop_back();
			subres_list_pos[storage_index] = -1;
		}
	}

	bool TiledResourceManager::is_subresource_mapped(uint array_slice, uint mip) const
	{
		return is_subresource_mapped_flat(flat_index(array_slice, mip));
	}

	bool TiledResourceManager::is_subresource_mapped_flat(uint index) const
	{
		return index < mapped_tile_count.size() && mapped_tile_count[index] != 0;
	}

}
