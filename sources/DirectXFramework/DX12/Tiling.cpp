#include "pch.h"

namespace DX12 {

	void TiledResourceManager::map_buffer_part(size_t offset, size_t size)
	{
		size_t begin = Math::AlignDown(offset, 64 * 1024) / (64 * 1024);
		size_t end = Math::AlignUp(offset + size, 64 * 1024) / (64 * 1024);
		map_buffer_range(begin, end);
	}

	void  TiledResourceManager::map_buffer_range(size_t from, size_t to)
	{
		assert(tiles.size() == 1);
		update_tiling_info info;
		info.resource = static_cast<Resource*>(this);

		for (size_t i = from; i < to; i++)
		{
			auto& tile = tiles[0][{i, 0, 0}];

			if (!tile.heap_position.heap)
			{
				tile.heap_position = ResourceHeapPageManager::get().create_tile(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, HeapType::DEFAULT);
				info.add_tile(tile);
			}
		}

		// TODO: make list
		Render::Device::get().get_queue(Render::CommandListType::DIRECT)->update_tile_mappings(info);
	}

	void TiledResourceManager::init_tilings()
	{
		UINT num_tiles = 1;
		D3D12_PACKED_MIP_INFO mip_info;
		D3D12_TILE_SHAPE tile_shape;
		UINT num_sub_res = 20;
		//  UINT first_sub_res;
		D3D12_SUBRESOURCE_TILING tilings[20];

		Device::get().get_native_device()->GetResourceTiling(static_cast<Resource*>(this)->get_native().Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);

		if (num_tiles > 0 && num_sub_res > 0)
		{
			tiles.resize(num_sub_res);
			for (UINT i = 0; i < num_sub_res; i++)
			{
				tiles[i].resize(uint3(tilings[i].WidthInTiles, tilings[i].HeightInTiles, tilings[i].DepthInTiles));

				for (int x = 0; x < tilings[i].WidthInTiles; x++)
					for (int y = 0; y < tilings[i].HeightInTiles; y++)
						for (int z = 0; z < tilings[i].DepthInTiles; z++)
							tiles[i][{x, y, z}].pos = { x,y,z };
			}
		}
	}
}