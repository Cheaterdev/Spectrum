export module Graphics:VSMPageTable;

import Core;
import :VSMClipmap;

export
{
	// Sentinel written to the indirection texture (and matched in shaders via
	// VSM_INVALID_SLOT) for a page that has no physical slot this frame --
	// unreachable while physical_page_count covers every level's pages, but
	// load-bearing once residency culling (Phase 5.4) lets demand exceed
	// supply.
	inline constexpr uint32_t VSM_INVALID_SLOT = 0xFFFFFFFFu;

	// A page's identity: which clip level, and its abs_page() grid coord
	// (stable across recentering -- see VSMClipmap::abs_page).
	struct VSMPageKey
	{
		int level = 0, x = 0, y = 0;
		bool operator==(const VSMPageKey&) const = default;
	};

	struct VSMPageKeyHash
	{
		size_t operator()(const VSMPageKey& k) const
		{
			size_t h = std::hash<int>()(k.level);
			h = h * 1000003u ^ std::hash<int>()(k.x);
			h = h * 1000003u ^ std::hash<int>()(k.y);
			return h;
		}
	};

	// CPU-authoritative layout of the physical atlas + page-table indirection.
	//
	// Real allocator (Phase 5.3): pages are keyed by identity (level +
	// abs_page), not by local slot position, so a page that stays resident
	// across a recenter keeps its physical slot and its rendered content --
	// the toroidal caching a fixed local->slot formula couldn't give.
	//
	// Physical slots are a free list; a page no longer required is expected
	// to be actively returned to it by VSM.cpp (evict_page(), called for
	// every not-needed cell every frame in plan_frame()), not discovered by
	// scanning for staleness here.
	//
	// Priority (Phase 5.5): when the free list is empty, allocate_or_touch
	// steals the least-recently-touched slot belonging to a level STRICTLY
	// COARSER than the requester -- never one equal or finer, so a page
	// near the camera can never be evicted to make room for one farther
	// away. This is only safe because every call happens from VSM::
	// plan_frame(), a single-threaded, once-per-frame pass that runs
	// entirely before any level's render() is dispatched (see VSM.ixx) --
	// stealing while renders were in flight is exactly what caused pages to
	// render on top of each other under load in an earlier version of this
	// allocator. If there's nothing coarser to steal from either,
	// allocate_or_touch fails closed (VSM_INVALID_SLOT); the shader's
	// get_vsm_slot() falls back to a coarser resident level instead of
	// showing a hole.
	class VSMPageTable
	{
	public:
		VSMClipmap clipmap;
		int page_size = 256;		// texels per page side
		// Physical pages the atlas can hold. The atlas is a Texture2DArray of
		// page_size^2 slices -- one slice per page, so a slot IS its slice and
		// there is no packed-region addressing.
		int physical_page_count = 49;

		struct AllocResult
		{
			uint32_t slot = VSM_INVALID_SLOT;
			bool newly_allocated = false;
		};

		// Returns this page's physical slot, allocating (from the free list,
		// or by stealing from a strictly coarser level -- see class comment)
		// if it isn't already resident. Touches last_used_frame either way.
		// Returns VSM_INVALID_SLOT (newly_allocated=false) if the free list
		// is empty and nothing coarser is resident to steal from.
		//
		// Caller must guarantee single-threaded, once-per-frame use (see
		// VSM::plan_frame) -- nothing here is synchronized.
		AllocResult allocate_or_touch(int level, ivec2 abs_page, uint64_t frame_id)
		{
			ensure_capacity();

			VSMPageKey key{ level, abs_page.x, abs_page.y };
			if (auto it = resident.find(key); it != resident.end())
			{
				last_used_frame[it->second] = frame_id;
				return { (uint32_t)it->second, false };
			}

			int slot = acquire_slot_with_priority(level);
			if (slot < 0)
				return {};

			resident[key]     = slot;
			slot_owner[slot]  = key;
			last_used_frame[slot] = frame_id;
			return { (uint32_t)slot, true };
		}

		// Returns a page's slot to the free list. Called for every page a
		// level decides it doesn't need this frame (see plan_frame) -- one
		// of two reclaim paths, not a fallback: since (level, abs_page) keys
		// are unique, only the one place that could currently ask for this
		// exact page is ever in a position to know it's not needed. No-op
		// if the page isn't resident.
		void evict_page(int level, ivec2 abs_page)
		{
			ensure_capacity();

			auto it = resident.find(VSMPageKey{ level, abs_page.x, abs_page.y });
			if (it == resident.end())
				return;

			int slot = it->second;
			resident.erase(it);
			slot_owner[slot] = VSMPageKey{ -1, 0, 0 };
			free_list.push_back(slot);
			pending_unmap_slots.push_back(slot);
		}

		// The other reclaim path: a page that has scrolled entirely off a
		// level's current grid (recentered past it) isn't covered by any of
		// that level's local cells any more, so evict_page() -- driven by
		// iterating THIS frame's grid -- never gets a chance to notice it.
		// Left alone, it's orphaned: still resident, still holding a slot,
		// but nothing will ever ask for that exact (level, abs_page) again.
		// For every level except the finest this is self-healing (a finer
		// level's allocate_or_touch can steal it, being strictly coarser),
		// but level 0 can never be stolen from -- its orphans are
		// permanent unless swept here, which is what caused pages to stop
		// allocating after enough camera movement (the pool slowly filling
		// with dead level-0 weight nothing could ever reclaim).
		//
		// Called once per level per frame in plan_frame() with that level's
		// current grid range; O(physical_page_count) scan, cheap at this
		// resident-set size.
		void evict_outside_range(int level, ivec2 min_incl, ivec2 max_excl)
		{
			for (auto it = resident.begin(); it != resident.end(); )
			{
				const VSMPageKey& k = it->first;
				bool outside = k.level == level &&
					(k.x < min_incl.x || k.x >= max_excl.x || k.y < min_incl.y || k.y >= max_excl.y);
				if (!outside)
				{
					++it;
					continue;
				}

				int slot = it->second;
				slot_owner[slot] = VSMPageKey{ -1, 0, 0 };
				free_list.push_back(slot);
				pending_unmap_slots.push_back(slot);
				it = resident.erase(it);
			}
		}

		// Which physical slots need a D3D12 tile-mapping change this frame,
		// for the reserved-resource atlas VSM owns directly. A slot only
		// needs an actual GPU map/unmap when it crosses the free-list
		// boundary (popped = needs backing memory, pushed = give its heap
		// tile back) -- a slot that changes owner via priority-stealing
		// stays physically mapped throughout, just gets redrawn.
		//
		// Called once per frame (from VSM_RenderPages' render(), before
		// issuing any tile-mapping GPU calls) to drain and coalesce this
		// frame's requests -- a slot freed then immediately re-acquired the
		// same frame nets out to "stays mapped," not two redundant GPU calls.
		void take_pending_tile_changes(std::vector<int>& out_to_map, std::vector<int>& out_to_unmap)
		{
			out_to_map.clear();
			out_to_unmap.clear();
			if (pending_map_slots.empty() && pending_unmap_slots.empty())
				return;

			std::unordered_set<int> unmap_set(pending_unmap_slots.begin(), pending_unmap_slots.end());
			std::unordered_set<int> map_set(pending_map_slots.begin(), pending_map_slots.end());

			for (int s : pending_map_slots)
				if (!unmap_set.count(s))
					out_to_map.push_back(s);
			for (int s : pending_unmap_slots)
				if (!map_set.count(s))
					out_to_unmap.push_back(s);

			pending_map_slots.clear();
			pending_unmap_slots.clear();
		}

	private:
		std::unordered_map<VSMPageKey, int, VSMPageKeyHash> resident;
		std::vector<int>      free_list;
		std::vector<VSMPageKey> slot_owner;
		std::vector<uint64_t> last_used_frame;
		std::vector<int> pending_map_slots;
		std::vector<int> pending_unmap_slots;

		void ensure_capacity()
		{
			if ((int)slot_owner.size() == physical_page_count)
				return;

			resident.clear();
			slot_owner.assign(physical_page_count, VSMPageKey{ -1, 0, 0 });
			last_used_frame.assign(physical_page_count, 0);
			free_list.clear();
			free_list.reserve(physical_page_count);
			for (int i = physical_page_count - 1; i >= 0; i--)
				free_list.push_back(i);
		}

		// Free list first; otherwise the least-recently-touched slot whose
		// owner is strictly coarser than `level` (never equal or finer).
		// Returns -1 if neither is available.
		int acquire_slot_with_priority(int level)
		{
			if (!free_list.empty())
			{
				int slot = free_list.back();
				free_list.pop_back();
				pending_map_slots.push_back(slot);
				return slot;
			}

			int victim = -1;
			uint64_t oldest = 0;
			for (int s = 0; s < physical_page_count; s++)
			{
				if (slot_owner[s].level <= level)
					continue;
				if (victim < 0 || last_used_frame[s] < oldest)
				{
					victim = s;
					oldest = last_used_frame[s];
				}
			}
			if (victim < 0)
				return -1;

			resident.erase(slot_owner[victim]);
			slot_owner[victim] = VSMPageKey{ -1, 0, 0 };
			return victim;
		}
	};
}
