#pragma once
#include "Math/Vectors.h"
#include "Memory.h"
namespace DX12
{
	class ResourceHeap;



	class TiledResourceManager
	{
		
	public://for now
		std::vector<grid<uint3, ResourceTile>> tiles;

		ResourceTile packed_tiles;
		UINT packed_mip_count;
		UINT unpacked_mip_count;

		UINT packed_subresource_offset = 0;
	protected:
		ivec3 tile_shape;
		//virtual	ComPtr<ID3D12Resource>& get_d3d_resource() = 0;
		void init_tilings();

		void load_tiles_internal(update_tiling_info& target, ivec3 from, ivec3 to, uint subres, bool recursive);
	

	public:
		void load_tile(update_tiling_info& target, ivec3 pos, uint subres, bool recursive);
		void zero_tile(update_tiling_info& target, ivec3 pos, uint subres);
		void copy_mappings(update_tiling_info& target, ivec3 target_pos, TiledResourceManager* source, ivec3 source_pos, ivec3 size);

		void load_packed(update_tiling_info& target);


		ivec3 get_tiles_count(int mip_level = 0);
		ivec3 get_tile_shape();
		void map_buffer_part(update_tiling_info& target, size_t offset, size_t size);

		void load_tiles(CommandList* list, ivec3 from, ivec3 to, uint subres = 0);
		void zero_tiles(CommandList* list, ivec3 from, ivec3 to);


		template<std::ranges::view R>
		void load_tiles2(CommandList* list, R tiles, uint subres = 0, bool recursive=false);

		void load_tiles(CommandList* list, std::list<ivec3>& tiles, uint subres = 0, bool recursive = false);
		void zero_tiles(CommandList* list, std::list<ivec3>& tiles);
		void zero_tiles(CommandList& list);

		inline bool is_mapped(ivec3 pos, uint subres)
		{
			return !!tiles[subres][pos].heap_position.heap;
		}

		void copy_mappings(CommandList& list, ivec3 target_pos, TiledResourceManager* source, ivec3 source_pos, ivec3 size);

		void map_tile(update_tiling_info&, ivec3 target_pos, TileHeapPosition pos);


		void load_packed(CommandList& list);

		Events::Event<uint4> on_load;
		Events::Event<uint4> on_zero;

	};
}