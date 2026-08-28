#include "Common.hlsl"

#include "autogen/tables/VSMConstants.h"
#include "autogen/tables/VSMLighting.h"

// Shared helpers only -- level/slot/tap resolution, nothing search- or
// resolve-specific. Split (Phase 5.18 follow-up) so that editing
// vsm_search_blocker (VSM_impl_search.hlsl, VSMBlockerSearchCompute's own
// file) doesn't force VSMApplyCompute (VSM.hlsl, VSM_impl_resolve.hlsl) to
// recompile, and vice versa -- this file is the only thing both actually
// share, so it's the only thing that should invalidate both.

// Matches VSMPageTable::VSM_INVALID_SLOT.
#define VSM_INVALID_SLOT 0xFFFFFFFFu

// level_info[] holds MaxLevels (26) slots, one geometric ladder, no
// regular/adaptive split (see VSMClipmap::page_world_size, VSM.ixx's
// LevelZeroSlot). Keep this literal 26 in sync with vsm.sig's level_info[26]
// if that ever changes -- only used as an array-bound sanity check now, the
// actual sweep range each frame is c.GetActive_min()/GetActive_max().
#define VSM_MAX_LEVELS 26

// Which clipmap level a world position falls into, using the shared
// light-space view + per-level grid_origin/page_world_size table VSM.cpp
// uploads every frame (VSMClipmap::grid_origin, computed identically on the
// CPU side so both agree on the same level for the same position).
// Returns -1 if pos_ls falls outside even the coarsest active level's grid
// (past the edge of the whole clipmap) -- callers must not clamp-sample in
// that case, since that would silently read an unrelated page.
//
// Phase 5.7: active levels are always a contiguous [active_min, active_max]
// range (see VSM.cpp's update_active_window()) -- finest (active_min) first,
// so the first containing level found is also the finest one covering this
// point.
int get_vsm_level(VSMConstants c, float2 pos_ls)
{
	int pages = c.GetPages_per_level();

	for (int level = c.GetActive_min(); level <= c.GetActive_max(); level++)
	{
		float4 info = c.GetLevel_info(level);
		float extent = info.z * pages;
		float2 rel = pos_ls - info.xy;
		if (all(rel >= 0) && all(rel < extent))
			return level;
	}
	return -1;
}

// Real allocator (Phase 5.3): a page in range isn't guaranteed resident (not
// yet reachable while every level's pages always fit the atlas, but becomes
// load-bearing once residency culling lets demand exceed supply -- Phase
// 5.4). On VSM_INVALID_SLOT, walk out to the next coarser level instead of
// returning unshadowed -- a blurrier shadow beats a hole.
//
// Phase 5.7: walk from start_level (whatever get_vsm_level found -- always
// within [active_min, active_max]) up to active_max, coarser levels only,
// since a coarser level's grid is a superset of a finer one's.
//
// resolved_level: which level the RETURNED slot actually belongs to --
// NOT necessarily start_level, whenever the walk above had to fall back to
// a coarser one. Callers that go on to look up c.GetLevel_info() for
// anything about the resolved page (texel size, page-grid extent, etc.)
// need THIS, not their own start_level -- using start_level there silently
// mixes one level's geometry with a different level's actual resolved
// page, which vsm_search_blocker's level_hiz fallback surfaced live as
// flatly wrong (not just imprecise) confident_lit/confident_dark bands
// exactly where residency happened to fall back to a coarser level.
uint get_vsm_slot(VSMConstants c, VSMLighting lighting, float2 pos_ls, int start_level, out int resolved_level)
{
	int pages = c.GetPages_per_level();
	resolved_level = start_level;

	for (int level = start_level; level <= c.GetActive_max(); level++)
	{
		float4 info = c.GetLevel_info(level);
		float2 page_f = (pos_ls - info.xy) / max(info.z, 0.0001);
		int2 page = clamp(int2(floor(page_f)), int2(0, 0), int2(pages - 1, pages - 1));

		uint slot = lighting.GetPage_table()[uint3(page.x, page.y, level)];
		if (slot != VSM_INVALID_SLOT)
		{
			resolved_level = level;
			return slot;
		}
	}
	return VSM_INVALID_SLOT;
}

// 2D rotation, used to spin the fixed Poisson disc per pixel below.
float2 vsm_rotate(float2 v, float angle)
{
	float s, c;
	sincos(angle, s, c);
	return float2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// Resolves which page a light-space XY position falls into, walking from
// the receiver's own level up through coarser levels on a miss -- mirrors
// get_vsm_slot's own fallback (a coarser level's grid is a superset, so it's
// more likely resident). VSM's allocator is dynamic and residency is
// partial by design, so a tap's neighbor page across an edge frequently
// just isn't loaded yet; giving up on that tap outright (as this used to)
// systematically undercounts blocker_count/penumbra taps right at whatever
// happens to be the edge of the currently-resident set -- a visible line
// that isn't a real scene feature, worse than "blurrier shadow beats a
// hole" reasoning already accepted for the receiver itself.
//
// Returns the page's atlas slice slot plus the position's local UV within
// it -- same X/Y-flip convention as get_shadow_vsm's light_tc (see the
// derivation there: UV.x tracks world X directly, UV.y = 1 - fractional
// world Y within the page).
bool vsm_resolve_tap(VSMConstants c, VSMLighting lighting, int level, float2 tap_pos_ls, out uint tap_slot, out float2 tap_uv)
{
	int pages = c.GetPages_per_level();

	[loop]
	for (int l = level; l <= c.GetActive_max(); l++)
	{
		float4 info = c.GetLevel_info(l);
		float2 page_f = (tap_pos_ls - info.xy) / max(info.z, 0.0001);
		int2 page = int2(floor(page_f));

		if (any(page < 0) || any(page >= pages))
			continue; // off this level's grid -- try a coarser (superset) one

		uint candidate = lighting.GetPage_table()[uint3(page.x, page.y, l)];
		if (candidate == VSM_INVALID_SLOT)
			continue;

		float2 local = page_f - float2(page);
		tap_slot = candidate;
		tap_uv = float2(local.x, 1.0 - local.y);
		return true;
	}

	tap_slot = VSM_INVALID_SLOT;
	tap_uv = 0;
	return false;
}

// One PCSS search/PCF tap. Works entirely in GLOBAL light-space (pos_ls),
// not page-local tc -- the offset (already in texels) is converted to a
// light-space delta via texel_world_size (radius in TEXELS * world-units-
// per-texel = world units; UV Y is flipped relative to light-space Y, see
// get_shadow_vsm's own derivation) and resolved through vsm_resolve_tap
// unconditionally, every tap, not just ones that cross the receiver's own
// page edge. Simpler and more uniform than special-casing "stayed in the
// receiver's page" as a separate fast path (that hybrid had its own local-tc
// math running alongside vsm_resolve_tap's independent global-tc math for
// different taps of the same pixel -- two formulations that were meant to
// agree but never got directly compared against each other, which is
// exactly the kind of split that hides a subtle inconsistency). One tap,
// one code path, one coordinate space.
bool vsm_tap(VSMConstants c, VSMLighting lighting, int level, float2 pos_ls,
             float texel_world_size, float2 rotated_offset, float radius_texels,
             out uint tap_slot, out float2 tap_uv)
{
	float2 tap_pos_ls = pos_ls + rotated_offset * radius_texels * texel_world_size * float2(1, -1);
	return vsm_resolve_tap(c, lighting, level, tap_pos_ls, tap_slot, tap_uv);
}

// Fixed Poisson disc, shared by the blocker search (VSM_impl_search.hlsl's
// vsm_search_blocker) and the PCF blur (VSM_ShadowResolve.hlsl's
// vsm_pcf_shadow) -- both unconditional now (no VSM_PENUMBRA define exists
// any more; use_vsm_penumbra gates which PassNodes even run, at the C++
// level, not an HLSL permutation) -- file scope so both can see it.
static const float2 VSM_POISSON_DISK[16] = {
	float2(-0.94201624, -0.39906216), float2( 0.94558609, -0.76890725),
	float2(-0.09418410, -0.92938870), float2( 0.34495938,  0.29387760),
	float2(-0.91588581,  0.45771432), float2(-0.81544232, -0.87912464),
	float2(-0.38277543,  0.27676845), float2( 0.97484398,  0.75648379),
	float2( 0.44323325, -0.97511554), float2( 0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023), float2( 0.79197514,  0.19090188),
	float2(-0.24188840,  0.99706507), float2(-0.81409955,  0.91437590),
	float2( 0.19984126,  0.78641367), float2( 0.14383161, -0.14100790),
};
