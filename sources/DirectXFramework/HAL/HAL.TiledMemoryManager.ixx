export module HAL:TiledMemeoryManager;

import Math;
import :Heap;
import :HeapAllocators;
import :Removeme;
import Events;
import stl.core;

export
{


	namespace HAL
	{

		
		struct update_tiling_info
		{
			std::map<Heap*, std::vector<ResourceTile>> tiles;
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
			Resource* resource;
		public://for now
			std::vector<grid<uint3, ResourceTile>> tiles;

			ResourceTile packed_tiles;
			UINT packed_mip_count;
			UINT unpacked_mip_count;

			UINT packed_subresource_offset = 0;

			std::vector<grid<uint3, ResourceTile>> gpu_tiles;
			ResourceTile gpu_packed_tile;

		protected:
			uint3 tile_shape;
			//virtual	ComPtr<ID3D12Resource>& get_d3d_resource() = 0;
			void load_tiles_internal(update_tiling_info& target, uint3 from, uint3 to, uint subres, bool recursive);

			static void commit(update_tiling_info& info, CommandList* list);

	
		public:		TiledResourceManager(Resource* resource) :resource(resource) {}void init_tilings();

			
			HAL::HeapType tile_heap_type = HAL::HeapType::DEFAULT;
			void load_tile(update_tiling_info& target, uint3 pos, uint subres, bool recursive);
			void zero_tile(update_tiling_info& target, uint3 pos, uint subres);
			void copy_mappings(update_tiling_info& target, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size);

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
				info.resource = resource;

				for (auto t : tiles)
					load_tile(info, t, subres, recursive);

				commit(info, list);

			}

			void load_tiles(CommandList* list, std::list<uint3>& tiles, uint subres = 0, bool recursive = false);
			void zero_tiles(CommandList* list, std::list<uint3>& tiles);
			void zero_tiles(CommandList& list);

			inline bool is_mapped(uint3 pos, uint subres) const
			{
				return !!tiles[subres][pos].heap_position.heap;
			}

			void copy_mappings(CommandList& list, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size);

			void map_tile(update_tiling_info&, uint3 target_pos, TileHeapPosition pos);


			void load_packed(CommandList& list);

			Events::Event<uint4> on_load;
			Events::Event<uint4> on_zero;


			void on_tile_update(const update_tiling_info& info);
		};
	}
}


