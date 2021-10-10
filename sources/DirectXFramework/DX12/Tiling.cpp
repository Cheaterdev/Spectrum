#include "pch_dx.h"
#include "Queue.h"

namespace DX12 {

	void TiledResourceManager::map_buffer_part(update_tiling_info& target, size_t offset, size_t size)
	{
		size_t begin = Math::AlignDown(offset, 64 * 1024) / (64 * 1024);
		size_t end = Math::AlignUp(offset + size, 64 * 1024) / (64 * 1024);

		load_tiles_internal(target, { begin,0,0 }, { end,0,0 }, 0, false);
		//load_tiles(target, { begin,0,0 }, { end,0,0 });
	}

	void TiledResourceManager::load_tile(update_tiling_info& target, uint3 pos, uint subres, bool recursive)
	{
		auto& alloc_info = static_cast<Resource*>(this)->alloc_info;
		auto& tile = tiles[subres][pos];

		if (!tile.heap_position.heap)
		{
			tile.heap_position = ResourceHeapPageManager::get().create_tile(alloc_info.flags, tile_heap_type);
			target.add_tile(tile);
			on_load(ivec4(pos,subres));
			if (recursive )
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
				
					if (math::all(pos >= uint3(0,0,0)) && math::all(pos < tiles[subres].size()))
					{
						return !!tiles[subres][pos].heap_position.heap;
					}

					return false;
				};

				bool can_unmap = true;

				for (int x = 0; x < 2; x++)
					for (int y = 0; y < 2; y++)
						for (int z = 0; z < 2; z++)
							can_unmap = can_unmap && !is_mapped(my_pos + uint3(x,y,z));

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
		info.resource = static_cast<Resource*>(this);
		load_tiles_internal(info, from, to, subres, true);
		// TODO: make list
		if (list)
		{
			list->update_tilings(std::move(info));
		}
		else
			Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::zero_tiles(CommandList* list, uint3 from, uint3 to)
	{

		update_tiling_info info;
		info.resource = static_cast<Resource*>(this);


		for (uint x = from.x; x <= to.x; x++)
			for (uint y = from.y; y <= to.y; y++)
				for (uint z = from.z; z <= to.z; z++)
				{
					zero_tile(info, {x,y,z}, 0);
				}

		// TODO: make list
		if (list)
		{
			list->update_tilings(std::move(info));
		}
		else
			Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}


	void TiledResourceManager::load_tiles(CommandList* list, std::list<uint3> &tiles, uint subres, bool recursive)
	{
		update_tiling_info info;
		info.resource = static_cast<Resource*>(this);

		for (auto t : tiles)
			load_tile(info, t, subres, recursive);


		// TODO: make list
		if (list)
		{
			list->update_tilings(std::move(info));
		}
		else
			Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}
	void TiledResourceManager::zero_tiles(CommandList* list, std::list<uint3> &tiles_to_remove)
	{
		update_tiling_info info;
		info.resource = static_cast<Resource*>(this);


			for(auto t: tiles_to_remove)
				{
					zero_tile(info, t, 0);
				}

		// TODO: make list
		if (list)
		{
			list->update_tilings(std::move(info));
		}
		else
			Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);

	}



	void TiledResourceManager::zero_tiles(CommandList& list)
	{
		update_tiling_info info;
		info.resource = static_cast<Resource*>(this);
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

		list.update_tilings(std::move(info));
	}


	void TiledResourceManager::copy_mappings(CommandList& list, uint3 target_pos, TiledResourceManager* source, uint3 source_pos, uint3 size)
	{
		update_tiling_info info;

		info.resource = static_cast<Resource*>(this);

		info.source = static_cast<Resource*>(source);
		info.source_pos = source_pos;
		info.pos = target_pos;
		info.size = size;
		list.update_tilings(std::move(info));
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
	void TiledResourceManager::init_tilings()
	{
		UINT num_tiles = 1;
		D3D12_PACKED_MIP_INFO mip_info;
		D3D12_TILE_SHAPE tile_shape;
		UINT num_sub_res = 20;
		//  UINT first_sub_res;
		D3D12_SUBRESOURCE_TILING tilings[20];

		auto resource = static_cast<Resource*>(this);

		auto desc = resource->get_desc();

		Device::get().get_native_device()->GetResourceTiling(resource->get_native().Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);
		packed_mip_count = mip_info.NumTilesForPackedMips;
		packed_subresource_offset = mip_info.NumStandardMips;
		unpacked_mip_count = mip_info.NumStandardMips;
		if (num_tiles > 0)
		{

			this->tile_shape = { tile_shape.WidthInTexels,tile_shape.HeightInTexels,tile_shape.DepthInTexels };

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				tiles.resize(1);
				tiles[0].resize(uint3(tilings[0].WidthInTiles, tilings[0].HeightInTiles, tilings[0].DepthInTiles));
				for (uint x = 0; x < tiles[0].size().x; x++)
							tiles[0][{x, 0, 0}].pos = { x,0,0 };

				resource->tracked_info->gpu_tiles.resize(1);
				resource->tracked_info->gpu_tiles[0].resize(uint3(tilings[0].WidthInTiles, tilings[0].HeightInTiles, tilings[0].DepthInTiles));
				for (uint x = 0; x < resource->tracked_info->gpu_tiles[0].size().x; x++)
					resource->tracked_info->gpu_tiles[0][{x, 0, 0}].pos = { x,0,0 };

			}
			else
			{
				tiles.resize(mip_info.NumStandardMips);
				resource->tracked_info->gpu_tiles.resize(mip_info.NumStandardMips);

				packed_tiles.pos = { 0,0,0 };
				packed_tiles.subresource = mip_info.NumStandardMips;
		

				for (UINT i = 0; i < mip_info.NumStandardMips; i++)
				{
					tiles[i].resize(uint3(tilings[i].WidthInTiles, tilings[i].HeightInTiles, tilings[i].DepthInTiles));
					resource->tracked_info->gpu_tiles[i].resize(uint3(tilings[i].WidthInTiles, tilings[i].HeightInTiles, tilings[i].DepthInTiles));

					for (uint x = 0; x < tiles[i].size().x; x++)
						for (uint y = 0; y < tiles[i].size().y; y++)
							for (uint z = 0; z < tiles[i].size().z; z++)

							{
								tiles[i][{x, y, z}].pos = { x,y,z };
								tiles[i][{x, y, z}].subresource = i;

								resource->tracked_info->gpu_tiles[i][{x, y, z}].pos = { x,y,z };
								resource->tracked_info->gpu_tiles[i][{x, y, z}].subresource = i;;
							}
				}
			}
			
		}
	}


	void TiledResourceManager::load_packed(CommandList& list)
	{
		if (packed_mip_count)
		{
			update_tiling_info info;
			info.resource = static_cast<Resource*>(this);
			auto& alloc_info = static_cast<Resource*>(this)->alloc_info;

			if (!packed_tiles.heap_position.heap)
				packed_tiles.heap_position = ResourceHeapPageManager::get().create_tile(alloc_info.flags, HeapType::DEFAULT, packed_mip_count);

			info.add_tile(packed_tiles);

			list.update_tilings(std::move(info));
		}
	}

	void TiledResourceManager::load_packed(update_tiling_info& info)
	{

		if (packed_mip_count)
		{
			auto& alloc_info = static_cast<Resource*>(this)->alloc_info;

			if (!packed_tiles.heap_position.heap)
				packed_tiles.heap_position = ResourceHeapPageManager::get().create_tile(alloc_info.flags, HeapType::DEFAULT, packed_mip_count);

			info.add_tile(packed_tiles);
		}
	}

	void TiledResourceManager::commit(update_tiling_info& info, CommandList* list)
	{
		// TODO: make list
		if (list)
		{
			list->update_tilings(std::move(info));
		}
		else
			Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
	}
}


