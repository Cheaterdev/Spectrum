#include "VSM_impl.hlsl"

#ifdef VSM_PENUMBRA
static const float VSM_SUN_ANGULAR_RADIUS = 0.02; // ~17 deg -- hardcoded, tune to taste.

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
	// A distant blocker can make penumbra_world huge, which shows up below as
	// the DENOMINATOR of every tap's w = depth_gap_world / penumbra_world --
	// growing it shrinks every tap's weight simultaneously, not just the ones
	// that should genuinely read as ambiguous. Push it far enough and sum_w
	// collapses under the "default to lit" threshold at the end of the tap
	// loop, so the shadow snaps from soft-but-present straight to fully lit
	// once world_delta crosses that point -- a visible sharp edge instead of
	// the gradual softening a growing penumbra should produce. Capping the
	// world-space size keeps w's denominator bounded, so confidence degrades
	// smoothly instead of falling off a cliff.
	static const float VSM_MAX_PENUMBRA_WORLD = 3.0; // world units, tune to taste.
	penumbra_world = min(penumbra_world, VSM_MAX_PENUMBRA_WORLD);
	float penumbra_texels = clamp(penumbra_world / texel_world_size, 1.0, c.GetPage_size() * 4.0);
 //pos_l_z*=1.001f;
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

		float scaler = 1-0.0001;
		float depth_gap_world = abs(pos_l_z*scaler - sampled) * depth_range;
		float w = saturate(depth_gap_world / max(penumbra_world, 0.00001));
		sum_w  += w;
		shadow += w * (sampled < pos_l_z*scaler);
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
	// Blocker-search extraction moved the quad-op safety concern (and the
	// `valid`-threading pattern that used to live here to protect it) into
	// vsm_search_blocker -- this function no longer does any quad ops
	// itself, so plain early returns are safe again.
	if (level < 0)
		return 1.0;

	int resolved_level = level;
	uint slot = get_vsm_slot(c, lighting, pos_ls, level, resolved_level);
	if (slot == VSM_INVALID_SLOT)
		return 1.0;
	// See get_vsm_slot's own comment: it can walk out to a coarser level
	// than the one just resolved above (residency fallback) -- from here
	// on, `level` means the level the resolved slot actually belongs to.
	level = resolved_level;

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

#ifdef VSM_PENUMBRA
	// PCSS: blocker search now runs in its OWN dispatch (VSM_BlockerSearch,
	// vsm_search_blocker) -- this reads its result back instead of
	// re-searching. Directional light, orthographic page cameras -- no
	// perspective divide, so this is NOT the textbook point/spot-light
	// formula penumbra=(recv-blocker)*lightSize/blocker (that assumes
	// projected footprint shrinks with light-space depth). For a
	// directional light penumbra grows linearly with world-space
	// blocker-to-receiver distance, scaled by the sun's angular radius.
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
	float texel_world_size = c.GetLevel_info(level).z / c.GetPage_size();

	// World-units-per-NDC-Z, from the page camera's own InvProj rather than
	// assuming a specific reversed-Z/orthographic matrix-element layout --
	// mirrors Common.hlsl's depth_to_wpos unprojection idiom. Needed by
	// vsm_pcf_shadow below regardless of which distance estimate ends up
	// driving a given blur pass.
	float4 vsm_depth_range_p0 = mul(page_cam.GetInvProj(), float4(0, 0, 0, 1));
	float4 vsm_depth_range_p1 = mul(page_cam.GetInvProj(), float4(0, 0, 1, 1));
	float depth_range = abs(vsm_depth_range_p1.z / vsm_depth_range_p1.w - vsm_depth_range_p0.z / vsm_depth_range_p0.w);

	// Rotate the whole disc by a per-pixel blue-noise angle -- 16 fixed taps
	// otherwise read as a rigid, repeating rosette once the radius gets big
	// (visible as concentric rings rather than a blur). blue_noise_texture is
	// baked once per frame at 128x128 (see BlueNoise.sig/blue_noise.hlsl) and
	// tiled here the same way every other consumer reads it (VoxelGI's
	// reflection reproject, etc.). Drives vsm_pcf_shadow's own (always
	// full-resolution, per-pixel) tap rotation below, and the full-penumbra
	// ray cone's jitter -- kept genuinely per-pixel for maximum
	// decorrelation, unlike vsm_search_blocker's own search_noise_angle.
	float noise_angle = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).x * 6.28318530718;

	// vsm_search_blocker's packed result (VSM_BlockerSearch's own dispatch,
	// see VSMLighting's blocker_result field for the exact uint4 layout).
	// Three sentinels, all from the Phase 5.18 Part A Hi-Z classification:
	// asuint(-1.0) = "no blocker found" (search disc entirely clear, or a
	// genuine empty search) -- skip straight to fully lit, same as the old
	// inline blocker_count==0 fast path. asuint(-2.0) = "confidently fully
	// shadowed" (whole search disc closer to light than the receiver) --
	// skip straight to fully dark. asuint(-3.0) = confident_lit specifically
	// (a SUBSET of the -1.0 case -- distinguishes "classification found
	// nothing" from "the real search never even ran to find anything",
	// which -1.0 alone can't tell apart) -- exists purely so
	// use_vsm_debug_hiz_classify (below) can visualize it; treated exactly
	// like -1.0 otherwise. Either way the expensive PCF pass below never
	// runs for any of the three.
	uint4 blocker_packed = lighting.GetBlocker_result()[pixel];
	float world_delta_or_sentinel = asfloat(blocker_packed.x);

	// Runtime debug toggle (VSM::use_vsm_debug_hiz_classify, VSMConstants'
	// debug_hiz_classify field): shows WHERE the Hi-Z classification is
	// firing and which way, instead of just its effect on the final
	// shadow. Signals back to combine_result via the same out-of-range-
	// sentinel trick get_shadow_vsm already uses for its own inputs --
	// -1.0 here means "show confident_lit" (green), -2.0 means "show
	// confident_dark" (blue); combine_result colors them and returns
	// early, so ambiguous/real-search pixels still shade normally for
	// context.
	bool debug_hiz = c.GetDebug_hiz_classify() != 0;

	// Bucketed most-negative-first and mutually exclusive -- -3.0 is ALSO
	// <= -1.5, so checking confident_dark's threshold first (as an earlier
	// version of this did) silently swallowed confident_lit into it
	// whenever debug_hiz was off: real-search-confirmed-lit areas rendered
	// fully shadowed instead. Confirmed live, fixed by checking the more
	// negative (more specific) sentinel first.
	//
	// TEMP DEBUG (live "does level_hiz fallback even fire" investigation):
	// -4.0/-5.0 are the via_level variants of -3.0/-2.0 (see
	// vsm_search_blocker's own comment) -- given distinct debug colors here
	// so the two pyramids' actual contribution is visible separately.
	// Functionally identical to their page-local counterparts outside
	// debug mode. Remove this pair of buckets once confirmed.
	if (world_delta_or_sentinel <= -4.5 && world_delta_or_sentinel > -5.5)
	{
		// confident_dark via level_hiz.
		if (debug_hiz)
			return -4.0;
		shadow = 0.0;
	}
	else if (world_delta_or_sentinel <= -3.5 && world_delta_or_sentinel > -4.5)
	{
		// confident_lit via level_hiz.
		if (debug_hiz)
			return -3.0;
		shadow = 1.0;
	}
	else if (world_delta_or_sentinel <= -2.5 && world_delta_or_sentinel > -3.5)
	{
		// confident_lit sentinel -- see this function's own comment above.
		if (debug_hiz)
			return -1.0;
		shadow = 1.0;
	}
	else if (world_delta_or_sentinel <= -1.5)
	{
		if (debug_hiz)
			return -2.0;
		// confident_dark sentinel -- see this function's own comment above.
		shadow = 0.0;
	}
	else if (world_delta_or_sentinel < 0)
	{
		// Nothing in the search disc occludes the light -- fully lit, skip
		// the penumbra PCF pass entirely.
		shadow = 1.0;
	}
	else
	{
		float  world_delta = world_delta_or_sentinel;
		float2 best_tc      = float2(asfloat(blocker_packed.y), asfloat(blocker_packed.z));
		uint   best_slot    = blocker_packed.w;

#ifdef VSM_RTX_VERIFY
		// Shadow maps only record the front-most surface per texel -- a
		// closer blocker can be geometrically present but never rasterized
		// into the atlas at the exact XY the search looked, so world_delta
		// above can understate the true occlusion.
		//
		// Rather than probing blindly along the sun direction (which
		// assumes the blocker sits exactly on that line -- wrong whenever
		// the winning tap had a lateral offset, which is most of them, by
		// construction of the Poisson disc), read the ACTUAL point the
		// shadow map flagged: unproject (best_tc, best_slot) -- that tap's
		// own page camera, since the winning tap may have resolved to a
		// different page/level than the receiver via vsm_resolve_tap's
		// coarser-level walk -- back into world space, and aim the
		// verification ray directly at that specific point. best_sampled_z
		// is re-sampled from the atlas here rather than carried through
		// vsm_search_blocker's packed uint4 -- it's just the depth already
		// stored at (best_tc, best_slot), one extra texture read is cheaper
		// than widening the packed result to a 5th channel.
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
			float best_sampled_z = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(best_tc, (float)best_slot), 0);
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
			shadow += lighting.GetVsm_atlas().SampleCmpLevelZero(vsmShadowSampler, float3(tc, (float)slot), pos_l.z*0.999);
		}
	}
	shadow /= 9.0;
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

	// Unused here (this debug helper never looks anything up by level after
	// resolving slot) -- required by get_vsm_slot's signature regardless.
	int unused_resolved_level = level;
	uint slot = get_vsm_slot(c, lighting, pos_ls, level, unused_resolved_level);
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
