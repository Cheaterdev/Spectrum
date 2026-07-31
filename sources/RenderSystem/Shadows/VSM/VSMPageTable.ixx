export module Graphics:VSMPageTable;

import Core;
import :VSMClipmap;

export
{
	// CPU-authoritative layout of the physical atlas + page-table indirection.
	//
	// Phase 1a: every (level, page) is always resident, mapped to its atlas
	// slot by a fixed identity formula (VSMClipmap::slot_of) -- there is no
	// eviction/caching to track yet, so there is no free-list/LRU here. That
	// bookkeeping (mark_requested/allocate_or_touch/evict_lru_if_needed from
	// the original design sketch) belongs to Phase 2, once pages can actually
	// go stale/cached across frames; adding it now would be unused surface.
	class VSMPageTable
	{
	public:
		VSMClipmap clipmap;
		int page_size = 256;			// texels per page side
		int atlas_pages_per_side = 7;	// >= ceil(sqrt(clipmap.total_pages()))

		int atlas_size_texels() const
		{
			return page_size * atlas_pages_per_side;
		}

		// Physical-atlas texel-space min corner of a page slot's region.
		ivec2 atlas_slot_origin(int slot) const
		{
			int sx = slot % atlas_pages_per_side;
			int sy = slot / atlas_pages_per_side;
			return ivec2(sx * page_size, sy * page_size);
		}

		// Flat (level, page) -> atlas slot indirection array, matching a
		// [pages_per_level x pages_per_level x level_count] Texture2DArray<uint>
		// layout (one texel per page, array slice = level).
		std::vector<uint32_t> build_indirection() const
		{
			std::vector<uint32_t> out(clipmap.total_pages());
			for (int level = 0; level < clipmap.level_count; level++)
				for (int py = 0; py < clipmap.pages_per_level; py++)
					for (int px = 0; px < clipmap.pages_per_level; px++)
					{
						int slot = clipmap.slot_of(level, ivec2(px, py));
						out[slot] = (uint32_t)slot;
					}
			return out;
		}
	};
}
