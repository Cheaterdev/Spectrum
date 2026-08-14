export module HAL:TiledMemoryManager;

import :Heap;
import :HeapAllocators;
import Core;

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

			// Phase 5.9 fix: a zero_tile() eviction's old ResourceHandle must not
			// be Free()'d until the GPU has actually finished with the tile it
			// was backing -- PagePool::free() (the allocator behind
			// ResourceHandle::Free()) releases the sub-allocation for immediate
			// reuse with no fence delay, so freeing here-and-now let a
			// completely unrelated allocation (observed: GUI label textures)
			// alias the same physical memory while VSM's reserved-resource
			// mapping was still live on the GPU timeline -- the "changing every
			// frame, looks like garbage/UI" symptom. Collected here instead and
			// actually freed in on_tile_update(), which only runs after
			// Queue::execute_internal's gpu_wait_thread has confirmed (via a
			// real fence wait) that this batch's GPU work has completed.
			std::vector<ResourceHandle> pending_free;

			void add_tile(ResourceTile tile);
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
			void load_tiles_internal(update_tiling_info& target, uint3 from, uint3 to, uint storage_index, bool recursive);

			static void commit(update_tiling_info& info, CommandList* list);

		public:
			TiledResourceManager(Resource* resource);
			void init_tilings();

			// Our own flat storage index into tiles[]/gpu_tiles[] for a given
			// (array_slice, mip) -- mip varies fastest, matching how
			// init_tilings() actually lays those arrays out. The one and only
			// place that combines "which slice" and "which mip" into a single
			// number; every public method below takes them separately instead
			// of asking callers to do this arithmetic themselves (which is
			// exactly how the old single "subres" parameter ended up meaning
			// a plain mip index for some callers and a mip-plus-slice combo
			// for others).
			uint flat_index(uint array_slice, uint mip) const
			{
				return mip + array_slice * unpacked_mip_count;
			}

			HAL::HeapType tile_heap_type = HAL::HeapType::DEFAULT;
			// Low-level, single-tile operations -- still take a raw storage
			// index (see flat_index() above) since the recursive coarser-mip
			// walk needs to increment it directly. Prefer the array_slice/mip
			// overloads of load_tiles()/zero_tiles() below unless you
			// specifically need one tile.
			void load_tile(update_tiling_info& target, uint3 pos, uint storage_index, bool recursive);
			void zero_tile(update_tiling_info& target, uint3 pos, uint storage_index);
			void copy_mappings(update_tiling_info& target, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size);

			void load_packed(update_tiling_info& target);

			bool is_tiled() const;

			uint3 get_tiles_count(uint array_slice = 0, uint mip = 0);
			uint3 get_tile_shape();
			void map_buffer_part(update_tiling_info& target, size_t offset, size_t size);

			void load_tiles(CommandList* list, uint3 from, uint3 to, uint array_slice = 0, uint mip = 0);
			// Unmaps a tile range for a given (array_slice, mip) -- one
			// signature covers both the "just slice 0" (VoxelGI's single-
			// slice usage) and "a specific slice/mip" (VSM's per-page-slot
			// atlas/HiZ) cases via defaults, instead of two overloads that
			// differ only in whether the caller hand-computed a combined
			// index.
			void zero_tiles(CommandList* list, uint3 from, uint3 to, uint array_slice = 0, uint mip = 0);


			template<std::ranges::view R>
			void load_tiles2(CommandList* list, R tiles, uint array_slice = 0, uint mip = 0, bool recursive = false)
			{
				update_tiling_info info;
				info.resource = resource;

				uint storage_index = flat_index(array_slice, mip);
				for (auto t : tiles)
					load_tile(info, t, storage_index, recursive);

				commit(info, list);

			}

			void load_tiles(CommandList* list, std::list<uint3>& tiles, uint array_slice = 0, uint mip = 0, bool recursive = false);
			void zero_tiles(CommandList* list, std::list<uint3>& tiles, uint array_slice = 0, uint mip = 0);
			void zero_tiles(CommandList& list);

			bool is_mapped(uint3 pos, uint array_slice, uint mip = 0) const;

			void copy_mappings(CommandList& list, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size);

			void map_tile(update_tiling_info&, uint3 target_pos, TileHeapPosition pos);


			void load_packed(CommandList& list);

			Events::Event<uint4> on_load;
			Events::Event<uint4> on_zero;


			void on_tile_update(update_tiling_info& info);
		};
	}
}



