#include "Common.hlsl"

#include "autogen/tables/VSMConstants.h"
#include "autogen/tables/VSMLighting.h"

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
uint get_vsm_slot(VSMConstants c, VSMLighting lighting, float2 pos_ls, int start_level)
{
	int pages = c.GetPages_per_level();

	for (int level = start_level; level <= c.GetActive_max(); level++)
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

#ifdef VSM_PENUMBRA
static const float VSM_SUN_ANGULAR_RADIUS = 0.02; // ~17 deg -- hardcoded, tune to taste.

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

// Confidence-weighted PCF blur (see the old inline version's comments in git
// history for the full weighting rationale), factored out of get_shadow_vsm
// so it can run TWICE against two independently-estimated blocker
// distances -- once for VSM's own shadow-map search, once for the
// RTX-verified distance -- instead of collapsing them into one blended
// world_delta before blurring a single time. See get_shadow_vsm's
// VSM_RTX_VERIFY branch for how the two resulting shadow values get
// combined (min(), not a blend) and why.
float vsm_pcf_shadow(VSMConstants c, VSMLighting lighting, int level, float2 pos_ls,
                      float pos_l_z, float texel_world_size, float depth_range,
                      float noise_angle, float world_delta)
{
	float penumbra_world = tan(VSM_SUN_ANGULAR_RADIUS) * max(world_delta, 0);
	float penumbra_texels = clamp(penumbra_world / texel_world_size, 1.0, c.GetPage_size() * 4.0);

	float shadow = 0;
	float sum_w = 0;
	[unroll]
	for (int si = 0; si < 16; si++)
	{
		uint tap_slot;
		float2 tc;
		// A failed resolve used to count as a confident LIT vote (full
		// weight), matching get_vsm_level/get_vsm_slot's own out-of-range
		// convention for "past the edge of the whole clipmap". But
		// vsm_resolve_tap also returns false for the much more common case
		// of a page that's simply not resident YET (ordinary caching/
		// residency churn at ordinary page boundaries, not the edge of the
		// world) -- with a wide search radius, MANY of the 16 taps for a
		// receiver near a page boundary can land in a not-yet-resident
		// neighbor at once, and confidently voting "lit" for every one of
		// them dragged the weighted average toward white right along page
		// boundaries (visible as a line). Neutral instead: contribute
		// nothing either way, so the result is driven only by taps that
		// actually found real data. The true "off the edge of the whole
		// world" case is still handled below, once, after the loop.
		if (!vsm_tap(c, lighting, level, pos_ls, texel_world_size,
		             vsm_rotate(VSM_POISSON_DISK[si], noise_angle), penumbra_texels,
		             tap_slot, tc))
			continue;
		float sampled = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(tc, (float)tap_slot), 0);

		float depth_gap_world = abs(pos_l_z*1.01 - sampled) * depth_range;
		float w = saturate(depth_gap_world / max(penumbra_world, 0.0001));
		sum_w  += w;
		shadow += w * (sampled < pos_l_z*1.01);
	}
	// sum_w stays exactly 0 only when literally every one of the 16 taps
	// failed to resolve -- the genuine "off the edge of the whole clipmap"
	// case (or every candidate landed in the self-shadow dead zone, which
	// only happens right where there's no real penumbra to speak of
	// anyway). Default to lit here, once, instead of per-tap.
	if (sum_w < 0.0001)
		return 1.0;
	return shadow / sum_w;
}
#endif

float get_shadow_vsm(VSMConstants c, VSMLighting lighting, float3 wpos, float3 normal, uint2 pixel)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
		return 1.0;

	uint slot = get_vsm_slot(c, lighting, pos_ls, level);
	if (slot == VSM_INVALID_SLOT)
		return 1.0;

	// VSMDepthDraw switched from cull=Front (render only back faces, so a
	// closed mesh's own front face never gets recorded as its own blocker)
	// to cull=None -- needed because cull=Front silently casts NO shadow at
	// all for single-sided/thin geometry (leaves, cards, thin walls) that
	// has no back face to rasterize. cull=None fixes that, but now a
	// front-facing receiver's own just-rasterized depth sits almost exactly
	// at its own reprojected pos_l.z, so ordinary rasterization/quantization
	// noise reads as self-shadowing (acne).
	//
	// A flat NDC-Z epsilon (tried first, both signs) is the wrong tool for
	// this: the PCSS blocker search reads texels up to
	// VSM_BLOCKER_SEARCH_RADIUS_WORLD away, and on any sloped/curved surface
	// the receiver's OWN depth varies across that radius by far more than a
	// small constant can safely absorb without either still acne-ing on
	// steep slopes or peter-panning on shallow ones. Standard fix instead:
	// push the shading point along its own surface NORMAL, in world space,
	// before reprojecting into light space -- geometry-based rather than
	// depth-based, so it naturally clears the receiver's own surface
	// regardless of slope. A plain fixed world-space distance, not scaled by
	// texel_world_size (was, briefly) -- that made the offset itself
	// level-dependent, one more moving part than this needs.
	static const float VSM_NORMAL_OFFSET_WORLD = 0.05; // world units, tune to taste.
	float3 biased_wpos = wpos + normal * VSM_NORMAL_OFFSET_WORLD;

	Camera page_cam = lighting.GetPage_cameras()[slot];
    float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));

	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	// The atlas is one array slice per page, so the slot IS the slice and the
	// page-local UV is used directly -- no packed-atlas offset math.
	// vsmShadowSampler (DefaultLayout's FrameLayout, HAL::Samplers::
	// SamplerShadowComparisonDesc) is a real SamplerComparisonState --
	// layout.jinja emits SamplerComparisonState instead of SamplerState for
	// any sampler whose desc identifier is SamplerShadowComparisonDesc
	// specifically (a name-based check, not a general SIG attribute -- see
	// hlsl/layout.jinja). GREATER_EQUAL matches this project's reversed-Z
	// convention (clear=0, closer fragments have larger stored z), so
	// SampleCmpLevelZero returns 1 (lit) when this point is at least as
	// close to the light as whatever is stored at that atlas texel -- same
	// polarity as the old manual `pos_l.z >= sampled` compare, now done by
	// the sampler's fixed-function hardware bilinear-PCF instead of a single
	// point sample.
	float2 texel_size = 1.0 / float2(c.GetPage_size(), c.GetPage_size());
	float shadow;
    int blocker_count = 0;

#ifdef VSM_PENUMBRA
	// PCSS: blocker search (sparse, wide radius, raw depth compares) then a
	// penumbra-scaled hardware-PCF using the same disc. Directional light,
	// orthographic page cameras -- no perspective divide, so this is NOT the
	// textbook point/spot-light formula penumbra=(recv-blocker)*lightSize/
	// blocker (that assumes projected footprint shrinks with light-space
	// depth). For a directional light penumbra grows linearly with
	// world-space blocker-to-receiver distance, scaled by the sun's angular
	// radius.
	//
	// A dense small-radius grid (the original cut of this) has two problems
	// at once: real occluders are usually farther than a couple of texels
	// from a penumbra-region receiver, so blocker search finds nothing and
	// silently falls back to shadow=1 without ever using the radius; and
	// even when it does fire, only 3 sample positions per axis spread across
	// a big radius reads as a handful of hard-edged blobs, not a blur. Fixed
	// by decoupling tap COUNT (kept small, fixed) from search RADIUS (can be
	// large) via a sparse Poisson disc -- the standard PCSS shape.
	// VSM_SUN_ANGULAR_RADIUS is now file-scope (see above vsm_tap) -- shared
	// with vsm_pcf_shadow, which needs it too.
	// World-space, not texel-space: a fixed texel count means a fine (small
	// page_world_size) page searches a tiny sliver of world space while a
	// coarse page searches a huge one, so real blockers fall outside the
	// window on fine pages and blocker_count silently comes back 0 right at
	// whatever level boundary the camera happens to be crossing -- a visible
	// line where the shadow flips from "fully lit" to a real penumbra.
	// Converted to a per-level texel count below via texel_world_size, same
	// as penumbra_texels already does.
	static const float VSM_BLOCKER_SEARCH_RADIUS_WORLD = 8.0;

	// page_world_size(level)/page_size -- world units per texel at whatever
	// level this pixel resolved to. Used for the blocker-search radius and
	// (below) for penumbra_texels; only needed under VSM_PENUMBRA, unlike
	// the normal-offset bias above which is now a plain constant.
	float texel_world_size = c.GetLevel_info(level).z / c.GetPage_size();

	// World-units-per-NDC-Z, from the page camera's own InvProj rather than
	// assuming a specific reversed-Z/orthographic matrix-element layout --
	// mirrors Common.hlsl's depth_to_wpos unprojection idiom. Hoisted this
	// far up (not just above the shadow-map blocker-search branch) because
	// vsm_pcf_shadow needs it too, regardless of whether world_delta or
	// the RTX-verified rtx_world_delta ends up driving a given blur pass.
	float4 vsm_depth_range_p0 = mul(page_cam.GetInvProj(), float4(0, 0, 0, 1));
	float4 vsm_depth_range_p1 = mul(page_cam.GetInvProj(), float4(0, 0, 1, 1));
	float depth_range = abs(vsm_depth_range_p1.z / vsm_depth_range_p1.w - vsm_depth_range_p0.z / vsm_depth_range_p0.w);

	// Texel-space radius the search disc actually samples at. radius_texels
	// * texel_world_size cancels back to exactly VSM_BLOCKER_SEARCH_RADIUS_
	// WORLD regardless of level -- THAT cancellation is the entire point of
	// dividing by texel_world_size in the first place. A tight ceiling here
	// (this used to clamp to page_size*0.125) breaks it: at the reference
	// level the raw ratio already exceeded that ceiling (128 texels wanted,
	// 64 allowed), so the clamp silently capped the ACHIEVED world-space
	// radius to half the target, and by a different, level-dependent amount
	// at every other level -- exactly backwards from the goal of a
	// level-independent search radius, and the actual cause of the radius
	// visibly depending on which level a pixel resolves to.
	//
	// No longer needed for correctness either way: capping the texel count
	// used to matter when taps just clamped uselessly at the current page's
	// edge, but vsm_resolve_tap now walks to whatever page/level genuinely
	// contains a tap's world position no matter how far that is. Only a
	// generous sanity ceiling remains, far past anything level_info's real
	// range should ever produce, plus a small floor so an extremely coarse
	// level can't round the search down to a useless sub-texel radius.
	float blocker_search_radius_texels = clamp(
		VSM_BLOCKER_SEARCH_RADIUS_WORLD / texel_world_size, 2.0, c.GetPage_size() * 4.0);

	// VSM_POISSON_DISK is now file-scope (see above vsm_tap) -- shared with
	// vsm_pcf_shadow, which needs the same disc for its own tap loop.

	// Rotate the whole disc by a per-pixel blue-noise angle -- 16 fixed taps
	// otherwise read as a rigid, repeating rosette once the radius gets big
	// (visible as concentric rings rather than a blur). blue_noise_texture is
	// baked once per frame at 128x128 (see BlueNoise.sig/blue_noise.hlsl) and
	// tiled here the same way every other consumer reads it (VoxelGI's
	// reflection reproject, etc.) -- .x and .y are independent noise values,
	// only .x is used since one angle covers both the blocker-search and PCF
	// passes below.
	float noise_angle = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).x * 6.28318530718;

	// "Min filter" idea (closest-to-light, so MAX under this project's
	// reversed-Z, not MIN) applied over the same already-sampled taps rather
	// than a separate precomputed pass: instead of averaging every found
	// blocker's depth, just take the single closest one. Cheap to test --
	// zero extra sampling -- but numerically this is the same substitution
	// as the earlier nearest-blocker attempt (reverted as "kinda bad"), so
	// expect a similar result; the article's real payoff (motion stability)
	// only comes from actually precomputing this in shadow-map space, which
	// this isn't doing.

	float max_blocker_z = 0;
	// Which tap VSM_RTX_VERIFY actually aims its ray at. Deliberately NOT
	// always max_blocker_z's own tap: a strict max is a stable, repeatable
	// winner across a local neighborhood of pixels whose rotated Poisson
	// discs overlap heavily, which is exactly the kind of texel-aligned
	// stair-stepping source that jittering the ray's final aim (tried
	// earlier) can't fix -- that jitter only wobbles a few cm around a
	// point that was already deterministically chosen, and mostly re-hits
	// the same surface. Randomizing WHICH near-tied candidate wins instead
	// changes the actual target position, which is the thing that needs to
	// vary. best_sampled_z is tracked separately from max_blocker_z (which
	// stays a strict max, unchanged, still driving world_delta's baseline
	// value below) because the two can now disagree -- best_tc/best_slot's
	// own depth is best_sampled_z, not necessarily max_blocker_z.
	float best_sampled_z = 0;
	float2 best_tc = 0;
	uint best_slot = 0;
	float best_pick_noise = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).y;
	// World-space NDC-Z tolerance for "close enough to the current max to
	// randomly swap in instead" -- converted via depth_range (hoisted
	// above) so this tunes in world units rather than an arbitrary
	// NDC-Z-range-dependent fraction.
	static const float VSM_RTX_TAP_TIE_EPS_WORLD = 0.1;
	[unroll]
	for (int bi = 0; bi < 16; bi++)
	{
		uint tap_slot;
		float2 tc;
		// Off the edge of the whole clip level (no page resolves there at
		// any level, even the coarsest) -- skip, don't count it either way.
		if (!vsm_tap(c, lighting, level, pos_ls, texel_world_size,
		             vsm_rotate(VSM_POISSON_DISK[bi], noise_angle), blocker_search_radius_texels,
		             tap_slot, tc))
			continue;
		// Raw depth, not hardware-compared -- vsmShadowSampler is a
		// SamplerComparisonState and can't be used with plain Sample*.
		float sampled = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(tc, (float)tap_slot), 0);
		// Reversed-Z: larger stored value = closer to light. A blocker is
		// anything closer to the light than this receiver. No self-shadow
		// dead zone here (unlike vsm_pcf_shadow) -- penumbra_world isn't
		// known yet at this point, it's what THIS search is still trying to
		// determine, so there's nothing to express a percentage of. Marginal
		// self-curvature candidates still get in as low-priority blockers,
		// but only matter downstream if vsm_pcf_shadow's own percentage-of-
		// penumbra dead zone doesn't already neutralize them there.
		if (sampled > pos_l.z)
		{
			bool is_new_max  = sampled > max_blocker_z || blocker_count == 0;
			bool is_near_tie = !is_new_max
				&& (max_blocker_z - sampled) * depth_range < VSM_RTX_TAP_TIE_EPS_WORLD
				&& best_pick_noise > 0.5;
			if (is_new_max || is_near_tie)
			{
				best_sampled_z = sampled;
				best_tc        = tc;
				best_slot      = tap_slot;
			}
			max_blocker_z = max(max_blocker_z, sampled);
			blocker_count++;
		}
	}

	if (blocker_count == 0)
	{
		// Nothing in the search disc occludes the light -- fully lit, skip
		// the penumbra PCF pass entirely.
		shadow = 1.0;
	}
	else
	{
		// depth_range computed once, hoisted above the blocker-search
		// branch -- shared with vsm_pcf_shadow below.
		//
		// Reversed-Z: a blocker satisfies sampled > pos_l.z by construction
		// (that's the only way it entered the search above), so
		// max_blocker_z - pos_l.z is the positive world-space distance the
		// closest blocker sits in front of the receiver. Had this backwards
		// originally (pos_l.z - avg_blocker_z) -- always negative when a
		// blocker exists, so max(..., 0) clamped it to exactly 0 every time,
		// making penumbra_texels floor to 1.0 unconditionally regardless of
		// the sun radius.
		float world_delta = (max_blocker_z - pos_l.z) * depth_range;

#ifdef VSM_RTX_VERIFY
		// Shadow maps only record the front-most surface per texel -- a
		// closer blocker can be geometrically present but never rasterized
		// into the atlas at the exact XY the search looked, so
		// max_blocker_z above can understate the true occlusion.
		//
		// Rather than probing blindly along the sun direction (which
		// assumes the blocker sits exactly on that line -- wrong whenever
		// the winning tap had a lateral offset, which is most of them, by
		// construction of the Poisson disc), read the ACTUAL point the
		// shadow map flagged: unproject (best_tc, best_sampled_z, best_slot)
		// -- that tap's own page camera, since the winning tap may have
		// resolved to a different page/level than the receiver via
		// vsm_resolve_tap's coarser-level walk -- back into world space,
		// and aim the verification ray directly at that specific point.
		// TMax is the distance to it plus a margin, not a tight bound: the
		// point is to give the ray room to find something EVEN CLOSER along
		// that same line of sight, not just re-confirm the target.
		//
		// rtx_world_delta is recomputed from wherever the ray actually
		// lands (dot(hit_pos - wpos, sun_dir), the hit projected onto the
		// true light axis) rather than substituted directly from
		// CommittedRayT() -- that raw T value is a distance along THIS
		// ray's own (generally non-axial) direction, not along the light,
		// so it isn't a valid drop-in replacement for the axial quantity
		// penumbra sizing needs.
		//
		// Deliberately NOT RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH (unlike
		// workgraph_test.hlsl's Shadows_Node) -- that flag returns whatever
		// the BVH happens to traverse first, not the closest hit, which
		// would make the result an unreliable distance. This needs the
		// genuinely closest hit within [TMin, TMax], so RayQuery is left to
		// run its normal closest-hit tracking to completion.
		static const float VSM_RTX_VERIFY_MARGIN = 1.5;
		bool  rtx_hit = false;
		float rtx_world_delta = world_delta;
		{
			Camera blocker_page_cam = lighting.GetPage_cameras()[best_slot];
			// Undo light_tc's own UV<->NDC mapping (see its derivation
			// above get_shadow_vsm's atlas sample): tc.x*2-1 un-does
			// *0.5+0.5, 1-tc.y*2 un-does *(-0.5)+0.5.
			float4 blocker_ndc  = float4(best_tc.x * 2 - 1, 1 - best_tc.y * 2, best_sampled_z, 1);
			float4 blocker_clip = mul(blocker_page_cam.GetInvViewProj(), blocker_ndc);
			float3 blocker_wpos = blocker_clip.xyz / blocker_clip.w;

			float3 sun_dir  = normalize(GetFrameInfo().GetSunDir().xyz);
			float3 to_target = blocker_wpos - wpos;
			float  target_dist = length(to_target);
			float3 aim_dir = target_dist > 0.0001 ? (to_target / target_dist) : sun_dir;

			// Blue noise .y (unused elsewhere -- noise_angle above only
			// consumes .x, so this stays decorrelated from the disc
			// rotation) dithers this otherwise perfectly deterministic
			// single-sample aim: a SMALL angular jitter around the exact
			// target direction, much narrower than the 16-ray mode's full
			// sun-disc cone -- this ray's job is still "verify one
			// specific point", not "sample the light disc". Smooths
			// texel-alignment stair-stepping in the reconstructed distance
			// across neighboring pixels (the winning tap snaps to a
			// discrete texel, so a perfectly deterministic ray inherits
			// that discreteness), and gives a bit of temporal variation
			// for free since the blue noise texture itself varies per
			// frame -- helps even without an explicit accumulation buffer.
			static const float VSM_RTX_VERIFY_JITTER = 0.02; // radians, small -- tune to taste.
			float3 jitter_up    = (abs(aim_dir.y) > 0.99) ? float3(1, 0, 0) : float3(0, 1, 0);
			float3 jitter_right = normalize(cross(jitter_up, aim_dir));
			jitter_up           = normalize(cross(aim_dir, jitter_right));
			float  jitter_angle = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).y * 6.28318530718;
			float2 jitter_offset = vsm_rotate(float2(1, 0), jitter_angle) * VSM_RTX_VERIFY_JITTER;
			float3 jittered_dir  = normalize(aim_dir + jitter_right * jitter_offset.x + jitter_up * jitter_offset.y);

			RayDesc ray;
			ray.Origin    = wpos + normal * 0.005;
			ray.Direction = jittered_dir;
			ray.TMin      = 0.01;
			ray.TMax      = max(target_dist * VSM_RTX_VERIFY_MARGIN, 0.02);

			RayQuery<RAY_FLAG_FORCE_OPAQUE> rayQuery;
			rayQuery.TraceRayInline(GetRaytracing().GetScene(), RAY_FLAG_NONE, 0xFF, ray);
			rayQuery.Proceed();

			if (rayQuery.CommittedStatus() != COMMITTED_NOTHING)
			{
				float3 hit_pos = ray.Origin + ray.Direction * rayQuery.CommittedRayT();
				rtx_world_delta = dot(hit_pos - wpos, sun_dir);
				rtx_hit = true;
			}
		}

		if (rtx_hit && c.GetRtx_dual_blur() != 0)
		{
			// Two independent blurs, not a blended world_delta: a single
			// blend means neither estimate is ever fully trusted, and a
			// case where only ONE method actually caught the true occluder
			// (VSM's search missed it per the front-most-surface limitation
			// above, or the verification ray's narrow TMax/jitter missed
			// something VSM's wider search disc still found) gets diluted
			// toward the wrong answer instead of winning outright. Running
			// vsm_pcf_shadow twice and taking min() -- shadow=0 is fully
			// occluded, shadow=1 is fully lit, under this function's
			// convention -- means whichever estimate found MORE shadow
			// always wins. Confirmed visually: removes bright spots that
			// used to appear between overlapping penumbras. Costs an extra
			// full 16-tap blur per pixel whenever the ray hits, so it's a
			// runtime toggle (VSM::use_vsm_rtx_dual_blur) rather than
			// unconditional.
			float shadow_vsm = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, world_delta);
			float shadow_rtx = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, rtx_world_delta);
			shadow = min(shadow_vsm, shadow_rtx);
		}
		else
		{
			// Single blur pass: use the RTX-verified distance when the ray
			// hit something (it's the more accurate estimate), otherwise
			// fall back to VSM's own shadow-map-derived distance.
			float chosen_delta = rtx_hit ? rtx_world_delta : world_delta;
			shadow = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, chosen_delta);
		}
#else
		shadow = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, world_delta);
#endif
	}
#else
	// Still averaged over a 3x3 grid of manually offset taps (Texture2DArray
	// has no SampleCmp overload taking a raw texel offset the way SampleLevel
	// does, so int2 offset here comes from re-deriving the UV per tap rather
	// than an offset param) -- four-tap hardware PCF per sample point, times
	// nine sample points, for a wider soft edge than a single hardware-PCF
	// tap alone would give.
	shadow = 0;
	[unroll]
	for (int oy = -1; oy <= 1; oy++)
	{
		[unroll]
		for (int ox = -1; ox <= 1; ox++)
		{
			float2 tc = light_tc + float2(ox, oy) * texel_size;
			shadow += lighting.GetVsm_atlas().SampleCmpLevelZero(vsmShadowSampler, float3(tc, (float)slot), pos_l.z*1.01);
		}
	}
	shadow /= 9.0;
	
    //shadow = blocker_count / 16.0;
#endif

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
