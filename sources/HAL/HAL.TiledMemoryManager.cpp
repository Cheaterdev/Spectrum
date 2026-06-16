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

	void TiledResourceManager::load_tile(update_tiling_info& target, uint3 pos, uint subres, bool recursive)
	{
		auto& alloc_info = (resource)->alloc_info;
		auto& tile = tiles[subres][pos];

		if (!tile.heap_position.heap)
		{
			tile.heap_position = resource->get_device().get_static_gpu_data().create_tile(tile_heap_type);
			target.add_tile(tile);
			on_load(ivec4(pos, subres));
			if (recursive)
			{
				if (subres != tiles.size() - 1)
				{
					load_tile(target, pos / 2, subres + 1, recursive);
				}
				else
				{
					load_packed(target);
				}
			}
		}
	}

	void  TiledResourceManager::zero_tile(update_tiling_info& target, uint3 pos, uint subres)
	{
		auto& tile = tiles[subres][pos];

		if (tile.heap_position.heap)
		{
			tile.heap_position.handle.Free();

			tile.heap_position.heap = nullptr;
			target.add_tile(tile);

			on_zero(ivec4(pos, subres));


			if (/*recursive && */subres != tiles.size() - 1)
			{
				uint3 parent_pos = pos / 2;

				uint3 my_pos = parent_pos * 2;

				auto is_mapped = [&](uint3 pos) {

					if (math::all(pos.higher_eq(uint3(0, 0, 0))) && math::all(pos.lower_eq(tiles[subres].size())))
					{
						return !!tiles[subres][pos].heap_position.heap;
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
					zero_tile(target, parent_pos, subres + 1);
				}

			}
		}
	}

	void TiledResourceManager::load_tiles_internal(update_tiling_info& target, uint3 from, uint3 to, uint subres, bool recursive)
	{
		for (uint x = from.x; x <= to.x; x++)
			for (uint y = from.y; y <= to.y; y++)
				for (uint z = from.z; z <= to.z; z++)
				{
					load_tile(target, { x,y,z }, subres, recursive);
				}
	}

	void TiledResourceManager::load_tiles(CommandList* list, uint3 from, uint3 to, uint subres)
	{

		update_tiling_info info;
		info.resource = resource;
		load_tiles_internal(info, from, to, subres, true);
		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::zero_tiles(CommandList* list, uint3 from, uint3 to)
	{

		update_tiling_info info;
		info.resource = resource;


		for (uint x = from.x; x <= to.x; x++)
			for (uint y = from.y; y <= to.y; y++)
				for (uint z = from.z; z <= to.z; z++)
				{
					zero_tile(info, { x,y,z }, 0);
				}

		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}


	void TiledResourceManager::load_tiles(CommandList* list, std::list<uint3>& tiles, uint subres, bool recursive)
	{
		update_tiling_info info;
		info.resource = resource;

		for (auto t : tiles)
			load_tile(info, t, subres, recursive);


		// TODO: make list
		if (list)
		{
			(list)->update_tilings(std::move(info));
		}
		else
			resource->get_device().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}
	void TiledResourceManager::zero_tiles(CommandList* list, std::list<uint3>& tiles_to_remove)
	{
		update_tiling_info info;
		info.resource = resource;


		for (auto t : tiles_to_remove)
		{
			zero_tile(info, t, 0);
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
			uint3 size = tiles[i].size();

			for (uint x = 0; x < tiles[i].size().x; x++)
				for (uint y = 0; y < tiles[i].size().y; y++)
					for (uint z = 0; z < tiles[i].size().z; z++)
					{
						zero_tile(info, { x, y, z }, 0);
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

		tile.heap_position = heap_pos;

		info.add_tile(tile);
	}

	uint3 TiledResourceManager::get_tiles_count(int mip_level)
	{
		return tiles[mip_level].size();

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

	void TiledResourceManager::on_tile_update(const update_tiling_info& info)
	{
		for (auto& [heap, tiles] : info.tiles)
		{
			for (auto tile : tiles)
			{
				if (tile.subresource == gpu_tiles.size())
					gpu_packed_tile = tile;
				else
					gpu_tiles[tile.subresource][tile.pos] = tile;
			}
		}
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

	bool TiledResourceManager::is_mapped(uint3 pos, uint subres) const
	{
		return !!tiles[subres][pos].heap_position.heap;
	}

}

