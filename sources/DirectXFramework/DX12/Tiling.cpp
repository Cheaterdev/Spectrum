#include "pch.h"

namespace DX12 {

	void TiledResourceManager::map_buffer_part( size_t offset, size_t size)
	{
		size_t begin =  Math::AlignDown(offset, 64 * 1024) / (64 * 1024);
		size_t end =  Math::AlignUp(offset + size, 64 * 1024) / (64 * 1024);

		for (size_t i = begin; i < end; i++)
		{
			auto& tile = tiles[0][i];

			if (!tile.heap_position.heap)
				map_buffer_tile(i);
		}

	}

	void  TiledResourceManager::map_buffer_tile(size_t x)
	{
		assert(tiles.size() == 1);

		auto resource = get_d3d_resource();
		auto& tile = tiles[0][x];

		tile.heap_position = ResourceHeapPageManager::get().create_tile(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, HeapType::DEFAULT);

		std::vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates;
		std::vector<D3D12_TILE_REGION_SIZE> regionSizes;
		std::vector<D3D12_TILE_RANGE_FLAGS> rangeFlags;
		std::vector<UINT> heapRangeStartOffsets;
		std::vector<UINT> rangeTileCounts;

		D3D12_TILED_RESOURCE_COORDINATE TRC;
		TRC.X = (UINT)x;
		TRC.Y = 0;
		TRC.Z = 0;
		TRC.Subresource = 0;

		D3D12_TILE_REGION_SIZE TRS;
		TRS.UseBox = false;
		TRS.Width = 1;
		TRS.Height = 1;
		TRS.Depth = 1;
		TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;

		startCoordinates.push_back(TRC);
		regionSizes.push_back(TRS);




		rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_NONE);
		heapRangeStartOffsets.push_back((UINT)tile.heap_position.offset);
		rangeTileCounts.push_back((UINT)TRS.NumTiles);


		/*update_tiling_info info;

		info.heap = tile.heap_position.heap->heap.Get();
		info.resource = resource.Get();

		info.startCoordinates = std::move(startCoordinates);
		info.regionSizes = std::move(regionSizes);
		info.rangeFlags = std::move(rangeFlags);
		info.heapRangeStartOffsets = std::move(heapRangeStartOffsets);
		info.rangeTileCounts = std::move(rangeTileCounts);




		list.update_tilings(std::move(info));
		*/

		Render::Device::get().get_queue(Render::CommandListType::DIRECT)->update_tile_mappings(
			resource.Get(),
			UINT(startCoordinates.size()),
			&startCoordinates[0],
			&regionSizes[0],
			tile.heap_position.heap->heap.Get(),
			UINT(rangeFlags.size()),
			&rangeFlags[0],
			&heapRangeStartOffsets[0],
			&rangeTileCounts[0],
			D3D12_TILE_MAPPING_FLAG_NONE
		);
		
	}

	void TiledResourceManager::init_tilings()
	{
		UINT num_tiles = 1;
		D3D12_PACKED_MIP_INFO mip_info;
		D3D12_TILE_SHAPE tile_shape;
		UINT num_sub_res = 20;
		//  UINT first_sub_res;
		D3D12_SUBRESOURCE_TILING tilings[20];

		auto resource = get_d3d_resource();
		Device::get().get_native_device()->GetResourceTiling(resource.Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);

		if (num_tiles > 0 && num_sub_res > 0)
		{
			tiles.resize(num_sub_res);
			for (UINT  i = 0; i < num_sub_res; i++)
				tiles[i].resize(tilings[i].DepthInTiles * tilings[i].WidthInTiles * tilings[i].HeightInTiles);


			/*
			int sr = 0;
			for (auto& subres : tiles)
			{
				int x = 0;
				for (auto& tile : subres)
				{
					tile.heap_position = ResourceHeapPageManager::get().create_tile(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);


					std::vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates;
					std::vector<D3D12_TILE_REGION_SIZE> regionSizes;
					std::vector<D3D12_TILE_RANGE_FLAGS> rangeFlags;
					std::vector<UINT> heapRangeStartOffsets;
					std::vector<UINT> rangeTileCounts;

					D3D12_TILED_RESOURCE_COORDINATE TRC;
					TRC.X = x;
					TRC.Y = 0;
					TRC.Z = 0;
					TRC.Subresource = sr;
					D3D12_TILE_REGION_SIZE TRS;
					TRS.UseBox = TRUE;
					TRS.Width = 1;
					TRS.Height = 1;
					TRS.Depth = 1;
					TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;
					startCoordinates.push_back(TRC);
					regionSizes.push_back(TRS);
					rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE);
					heapRangeStartOffsets.push_back(tile.heap_position.offset);
					rangeTileCounts.push_back(TRS.NumTiles);


					Render::Device::get().get_queue(Render::CommandListType::DIRECT)->update_tile_mappings(
						m_Resource.Get(),
						UINT(startCoordinates.size()),
						&startCoordinates[0],
						&regionSizes[0],
						tile.heap_position.heap->heap.Get(),
						UINT(startCoordinates.size()),
						&rangeFlags[0],
						&heapRangeStartOffsets[0],
						&rangeTileCounts[0],
						D3D12_TILE_MAPPING_FLAG_NONE
					);


					x++;
				}

				sr++;
			}
			*/

		

		}

	}


}