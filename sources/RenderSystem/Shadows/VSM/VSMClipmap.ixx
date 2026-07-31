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

		int total_pages() const
		{
			return level_count * pages_per_level * pages_per_level;
		}

		// Flat slot index for (level, page). Deterministic 1:1 mapping in
		// Phase 1a -- dynamic allocation/eviction is Phase 2.
		int slot_of(int level, ivec2 page) const
		{
			return (level * pages_per_level + page.y) * pages_per_level + page.x;
		}
	};
}
