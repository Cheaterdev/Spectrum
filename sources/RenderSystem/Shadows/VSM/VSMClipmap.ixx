export module Graphics:VSMClipmap;

import Core;

export
{
	// Pure math for a directional-light page clipmap: level -> world-space
	// page size, and the world-space (light-view-space XY) origin of a
	// level's page grid given where the camera currently is.
	//
	// Phase 1a note: the (level, page) index space is fixed (level_count *
	// pages_per_level^2 pages, always all considered "needed" -- no per-page
	// frustum culling or caching yet, that is Phase 1b/2/3). Only the world
	// meaning of each page index moves, via grid_origin() re-centering on the
	// camera every frame.
	class VSMClipmap
	{
	public:
		int level_count = 3;
		int pages_per_level = 4;
		float base_page_world_size = 8.0f;

		float page_world_size(int level) const
		{
			return base_page_world_size * float(1 << level);
		}

		// Snapped, camera-centered world-space min corner of the level's
		// pages_per_level x pages_per_level page grid, in light-view-space XY.
		float2 grid_origin(int level, float2 cam_pos_ls) const
		{
			float size = page_world_size(level);
			float2 snapped = float2(std::floor(cam_pos_ls.x / size), std::floor(cam_pos_ls.y / size)) * size;
			return snapped - float2((float)(pages_per_level / 2)) * size;
		}

		float2 page_min(int level, ivec2 page, float2 origin) const
		{
			return origin + float2((float)page.x, (float)page.y) * page_world_size(level);
		}

		// Inverse of page_min: which page a light-view-space XY position
		// falls into, clamped to the valid [0, pages_per_level) range (a
		// point outside the level's grid maps to the nearest edge page --
		// used for per-object invalidation, where "clamp to the edge" is a
		// safe, if slightly conservative, fallback rather than a hard error).
		ivec2 world_to_page(int level, float2 pos_ls, float2 origin) const
		{
			float size = page_world_size(level);
			int px = (int)std::floor((pos_ls.x - origin.x) / size);
			int py = (int)std::floor((pos_ls.y - origin.y) / size);
			px = std::clamp(px, 0, pages_per_level - 1);
			py = std::clamp(py, 0, pages_per_level - 1);
			return ivec2(px, py);
		}

		int total_pages() const
		{
			return level_count * pages_per_level * pages_per_level;
		}

		// A page's identity independent of the grid's current recentering:
		// origin is always an exact multiple of page_world_size(level) (see
		// grid_origin), so origin/size is an integer grid index -- adding the
		// local page coord gives a key that stays the same for the same
		// world-space page across recenters, letting VSMPageTable cache pages
		// by identity instead of by local slot position.
		ivec2 abs_page(int level, ivec2 local, float2 origin) const
		{
			float size = page_world_size(level);
			ivec2 grid_idx((int)std::lround(origin.x / size), (int)std::lround(origin.y / size));
			return grid_idx + local;
		}
	};
}
