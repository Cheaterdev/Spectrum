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

			// Mapped-subresource mask: lets barrier-tracking code for a
			// reserved/Virtual resource (e.g. VSM_Atlas: 2048 declared array
			// slices, ~256 ever tile-mapped) skip subresources nobody has
			// backed with real memory instead of walking every declared one.
			//
			// mapped_tile_count[flat_index] = how many of that subresource's
			// tiles are currently mapped -- one counter per tiles[] entry,
			// maintained incrementally by load_tile()/zero_tile()/map_tile()
			// (each already gates on the tile's own mapped/unmapped state
			// before touching it, so the counter can never double-count).
			// is_subresource_mapped() is then an O(1) count!=0 read, no scan.
			//
			// mapped_subresources is the same fact kept as a compact, order-
			// preserving list of flat_index values with count>0 -- lets a
			// caller enumerate just the working set (~256 entries) instead of
			// the full declared range (2048) when it needs to VISIT every
			// mapped subresource, not just test one. subres_list_pos mirrors
			// it (flat_index -> position in mapped_subresources, or -1) so
			// add/remove during load_tile/zero_tile are O(1) swap-erase
			// instead of a linear search.
			std::vector<uint32_t> mapped_tile_count;
			std::vector<UINT>     mapped_subresources;
			std::vector<int>      subres_list_pos;

			// 0->1 / 1->0 transition bookkeeping for the mask above -- called
			// from load_tile()/zero_tile()/map_tile() exactly where each
			// already knows a tile just became mapped/unmapped. Lazily grows
			// the mask vectors to tiles.size() on first use, since tiles[]
			// itself is only sized once init_tilings() (backend-specific) has
			// run.
			void mark_tile_mapped(uint storage_index);
			void mark_tile_unmapped(uint storage_index);

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

			// Phase 5.10: same range as load_tiles()/zero_tiles() above, but
			// applied to MULTIPLE array slices in one update_tiling_info and
			// one commit()/update_tilings() call, instead of one call per
			// slice. update_tile_mappings() (HAL.Queue.cpp) issues one real
			// ID3D12CommandQueue::UpdateTileMappings per update_tiling_info
			// it drains -- calling load_tiles()/zero_tiles() once per slot in
			// a loop means one native call per slot; these two exist so a
			// caller batching many slots' worth of (un)mapping in the same
			// frame (VSM's per-frame tile-mapping drain) pays for exactly one.
			void load_tiles_batch(CommandList* list, uint3 from, uint3 to, const std::vector<int>& array_slices, uint mip = 0);
			void zero_tiles_batch(CommandList* list, uint3 from, uint3 to, const std::vector<int>& array_slices, uint mip = 0);


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

			// True iff subresource (array_slice, mip) has at least one tile
			// currently mapped to real memory. See mapped_tile_count above.
			bool is_subresource_mapped(uint array_slice, uint mip = 0) const;

			// Same as above but takes flat_index()'s own raw index directly --
			// equals the real D3D12 subresource index whenever this resource
			// has no packed mip tail (mip count == unpacked_mip_count), which
			// covers every current Virtual-resource consumer in this codebase
			// (see flat_index()'s own comment). Barrier-tracking code (which
			// deals in real D3D12 subresource indices) should call this
			// overload directly rather than decompose subres into slice/mip
			// itself.
			bool is_subresource_mapped_flat(uint index) const;

			// Compact, order-preserving list of every currently-mapped
			// flat_index -- iterate this instead of [0, declared_count) when
			// visiting every mapped subresource of a reserved resource.
			const std::vector<UINT>& get_mapped_subresources() const { return mapped_subresources; }

			void copy_mappings(CommandList& list, uint3 target_pos, Resource* source, uint3 source_pos, uint3 size);

			void map_tile(update_tiling_info&, uint3 target_pos, TileHeapPosition pos);


			void load_packed(CommandList& list);

			Events::Event<uint4> on_load;
			Events::Event<uint4> on_zero;


			void on_tile_update(update_tiling_info& info);
		};
	}
}



