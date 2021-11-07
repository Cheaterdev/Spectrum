module;
#include "pch_dx.h"

#include "dx12_types.h"
export module Tiling;

import Vectors;
import Memory;
import Events;
import stl.core;

import HAL.Types;
using namespace HAL;

export
{


	namespace DX12
	{


		struct update_tiling_info
		{
			std::map<ResourceHeap*, std::vector<ResourceTile>> tiles;
			Resource* resource = nullptr;

			Resource* source = nullptr;
			ivec3 pos;
			ivec3 source_pos;
			ivec3 size;
			uint source_subres;
			uint target_subres;

			void add_tile(ResourceTile tile)
			{
				tiles[tile.heap_position.heap.get()].push_back(tile);
			}
		};

		class TiledResourceManager
		{

		public://for now
			std::vector<grid<uint3, ResourceTile>> tiles;

			ResourceTile packed_tiles;
			UINT packed_mip_count;
			UINT unpacked_mip_count;

			UINT packed_subresource_offset = 0;
		protected:
			uint3 tile_shape;
			//virtual	ComPtr<ID3D12Resource>& get_d3d_resource() = 0;
			void init_tilings();

			void load_tiles_internal(update_tiling_info& target, uint3 from, uint3 to, uint subres, bool recursive);

			static void commit(update_tiling_info& info, CommandList* list);
		public:
			HeapType tile_heap_type = HeapType::DEFAULT;
			void load_tile(update_tiling_info& target, uint3 pos, uint subres, bool recursive);
			void zero_tile(update_tiling_info& target, uint3 pos, uint subres);
			void copy_mappings(update_tiling_info& target, uint3 target_pos, TiledResourceManager* source, uint3 source_pos, uint3 size);

			void load_packed(update_tiling_info& target);


			uint3 get_tiles_count(int mip_level = 0);
			uint3 get_tile_shape();
			void map_buffer_part(update_tiling_info& target, size_t offset, size_t size);

			void load_tiles(CommandList* list, uint3 from, uint3 to, uint subres = 0);
			void zero_tiles(CommandList* list, uint3 from, uint3 to);


			template<std::ranges::view R>
			void load_tiles2(CommandList* list, R tiles, uint subres = 0, bool recursive = false)
			{
				update_tiling_info info;
				info.resource = static_cast<Resource*>(this);

				for (auto t : tiles)
					load_tile(info, t, subres, recursive);

				commit(info, list);

			}

			void load_tiles(CommandList* list, std::list<uint3>& tiles, uint subres = 0, bool recursive = false);
			void zero_tiles(CommandList* list, std::list<uint3>& tiles);
			void zero_tiles(CommandList& list);

			inline bool is_mapped(uint3 pos, uint subres)
			{
				return !!tiles[subres][pos].heap_position.heap;
			}

			void copy_mappings(CommandList& list, uint3 target_pos, TiledResourceManager* source, uint3 source_pos, uint3 size);

			void map_tile(update_tiling_info&, uint3 target_pos, TileHeapPosition pos);


			void load_packed(CommandList& list);

			Events::Event<uint4> on_load;
			Events::Event<uint4> on_zero;

		};
	}
}