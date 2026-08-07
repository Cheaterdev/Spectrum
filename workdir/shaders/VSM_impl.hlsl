#include "Common.hlsl"

#include "autogen/tables/VSMConstants.h"
#include "autogen/tables/VSMLighting.h"

// Matches VSMPageTable::VSM_INVALID_SLOT.
#define VSM_INVALID_SLOT 0xFFFFFFFFu

// level_info[] holds MaxLevels (9) slots (see vsm.sig's VSMConstants) --
// c.GetLevel_count() is only the REGULAR ring count (VSMClipmap::
// regular_level_count, currently 6); indices count..8 are the Phase 5.6
// adaptive tiers, finer than level 0, addressed by fixed index rather than
// folded into that loop bound. Keep this literal 9 in sync with vsm.sig's
// level_info[9] if that ever changes.
#define VSM_MAX_LEVELS 9

// Which clipmap level a world position falls into, using the shared
// light-space view + per-level grid_origin/page_world_size table VSM.cpp
// uploads every frame (VSMClipmap::grid_origin, computed identically on the
// CPU side so both agree on the same level for the same position).
// Returns -1 if pos_ls falls outside even the coarsest level's grid (past
// the edge of the whole clipmap) -- callers must not clamp-sample in that
// case, since that would silently read an unrelated page.
int get_vsm_level(VSMConstants c, float2 pos_ls)
{
	int pages = c.GetPages_per_level();

	// Adaptive tiers first, deepest (finest) active one wins -- they cover
	// strictly less area than level 0, so a point they don't contain still
	// needs the regular sweep below regardless of activity.
	for (int adaptive = VSM_MAX_LEVELS - 1; adaptive >= c.GetLevel_count(); adaptive--)
	{
		float4 info = c.GetLevel_info(adaptive);
		if (info.w == 0) // inactive
			continue;
		float extent = info.z * pages;
		float2 rel = pos_ls - info.xy;
		if (all(rel >= 0) && all(rel < extent))
			return adaptive;
	}

	for (int level = 0; level < c.GetLevel_count(); level++)
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
// start_level can be an adaptive tier (>= c.GetLevel_count()) -- walk
// COARSER adaptive tiers first (decreasing index, toward c.GetLevel_count()
// -- deeper/finer tiers are higher indices, see get_vsm_level), then fall
// into the regular sweep from level 0. The regular for-loop's own bound
// (c.GetLevel_count()) means it would never execute at all if start_level
// was left at an adaptive index, silently skipping every regular level.
uint get_vsm_slot(VSMConstants c, VSMLighting lighting, float2 pos_ls, int start_level)
{
	int pages = c.GetPages_per_level();

	if (start_level >= c.GetLevel_count())
	{
		for (int adaptive = start_level; adaptive >= c.GetLevel_count(); adaptive--)
		{
			float4 info = c.GetLevel_info(adaptive);
			if (info.w == 0)
				continue;
			float2 page_f = (pos_ls - info.xy) / max(info.z, 0.0001);
			int2 page = clamp(int2(floor(page_f)), int2(0, 0), int2(pages - 1, pages - 1));
			uint slot = lighting.GetPage_table()[uint3(page.x, page.y, adaptive)];
			if (slot != VSM_INVALID_SLOT)
				return slot;
		}
		start_level = 0;
	}

	for (int level = start_level; level < c.GetLevel_count(); level++)
	{
		float4 info = c.GetLevel_info(level);
		float2 page_f = (pos_ls - info.xy) / max(info.z, 0.0001);
		int2 page = clamp(int2(floor(page_f)), int2(0, 0), int2(pages - 1, pages - 1));

		uint slot = lighting.GetPage_table()[uint3(page.x, page.y, level)];
		if (slot != VSM_INVALID_SLOT)
			return slot;
	}
	return VSM_INVALID_SLOT;
}

float get_shadow_vsm(VSMConstants c, VSMLighting lighting, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return 1.0;

	uint slot = get_vsm_slot(c, lighting, pos_ls, level);
	if (slot == VSM_INVALID_SLOT)
		return 1.0;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	// The atlas is one array slice per page, so the slot IS the slice and the
	// page-local UV is used directly -- no packed-atlas offset math.
	// No comparison sampler is declared in DefaultLayout's sampler set, so
	// this does the depth test manually rather than via SampleCmp -- a plain
	// 3x3 box of manually-compared texel-offset taps (SampleLevel's built-in
	// int2 offset param) instead of PSSM_impl.hlsl's dilated-Poisson layout,
	// whose spread constants are dead/commented-out there and collapse to
	// duplicate offsets if copied literally. Reversed-Z convention (clear=0,
	// closer fragments have larger stored z): lit if this point is at least
	// as close to the light as whatever is stored at that atlas texel.
	float shadow = 0;
	[unroll]
	for (int oy = -1; oy <= 1; oy++)
	{
		[unroll]
		for (int ox = -1; ox <= 1; ox++)
		{
			float sampled = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(light_tc, (float)slot), 0, int2(ox, oy));
			shadow += (pos_l.z >= sampled) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (pos_l.z < 0 || pos_l.z > 1 || any(light_tc < 0) || any(light_tc > 1))
		shadow = 1;

	return shadow;
}

// Debug: raw sampled atlas depth at the page this world pos maps to (0 if
// out of range). Reversed-Z, clear=0 -- if this is 0 everywhere, nothing was
// ever rasterized into the atlas at that page; a real surface written there
// will read as a small-to-large positive value depending on distance from
// the light.
float get_vsm_debug_raw_depth(VSMConstants c, VSMLighting lighting, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return 0;

	uint slot = get_vsm_slot(c, lighting, pos_ls, level);
	if (slot == VSM_INVALID_SLOT)
		return 0;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));
	pos_l /= pos_l.w;
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	return lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(light_tc, (float)slot), 0);
}

// Debug heatmap: one flat color per clipmap level, for visually confirming
// page/level coverage (Phase 1a "done" criterion).
float3 get_vsm_debug_color(VSMConstants c, float3 wpos)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return float3(0, 0, 0);

	static const float3 palette[8] = {
		float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(1, 1, 0),
		float3(0, 1, 1), float3(1, 0, 1), float3(1, 0.5, 0), float3(0.5, 0, 1)
	};
	return palette[level % 8];
}
