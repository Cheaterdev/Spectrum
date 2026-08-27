#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMTileListRead.h"
#include "autogen/VSMShadowResolveIO.h"
// Only actually referenced inside CS_SHADOW_BLUR's VSM_RTX_VERIFY branch;
// unconditionally included since it's dead-code-eliminated when the define
// is off, same as every other permutation-gated accessor in this codebase.
#include "autogen/Raytracing.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

// Shared level/slot/tap helpers (get_vsm_level, get_vsm_slot, vsm_tap,
// vsm_rotate, VSM_POISSON_DISK) -- NOT VSM_impl_search.hlsl (this file never
// needs vsm_search_blocker/vsm_classify_blocker; stages 1/2 already did that
// work) and NOT VSM_impl_resolve.hlsl (that file's own get_shadow_vsm is
// what this file replaces -- see VSM.hlsl's combine_result for the
// simplified, post-refactor caller).
#include "VSM_impl.hlsl"

// Stage 3 (Phase 5.18 Part A follow-up, take 4): three PSOs sharing this one
// file, one PassNode, one render() -- see vsm.sig's VSM_ShadowResolve
// PassNode comment for why all three must be issued from the same render()
// (mirrors VoxelGIGraph's VoxelCombine issuing its own blur+blur2
// exec_indirects together -- the root-cause fix for two separate
// completely-black-screen bugs earlier this session).
//
// Common tile-to-pixel reconstruction shared by all three entry points --
// SV_GroupID.x indexes whichever tile list this dispatch was built from
// (VSMBlockerClassifyInitDispatch sized each indirect dispatch to exactly
// that list's appended count, one 16x16 group per tile).
uint2 resolve_pixel(uint3 groupID, uint3 groupThreadID)
{
	return GetVSMTileListRead().GetTiles()[groupID.x] * 16 + groupThreadID.xy;
}

// full-lit: stage 1 already proved every pixel in this tile is confidently
// lit -- no per-pixel check, no atlas sampling, just write the answer.
[numthreads(16, 16, 1)]
void CS_FULL_LIT(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);
	uint2 pixel = resolve_pixel(groupID, groupThreadID);
	if (all(pixel < dims))
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 1.0;
}

// full-shadow: same as full-lit, opposite verdict.
[numthreads(16, 16, 1)]
void CS_FULL_SHADOW(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);
	uint2 pixel = resolve_pixel(groupID, groupThreadID);
	if (all(pixel < dims))
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 0.0;
}

static const float VSM_SUN_ANGULAR_RADIUS = 0.02; // ~17 deg -- hardcoded, tune to taste.

// Confidence-weighted PCF blur (see the old inline version's comments in git
// history for the full weighting rationale) -- moved here verbatim from the
// old get_shadow_vsm/VSM_impl_resolve.hlsl (Phase 5.18 Part A follow-up,
// take 4): this PSO is what now actually owns the blur, since stage 2 only
// ever finds the blocker distance, it doesn't blur. Runs TWICE against two
// independently-estimated blocker distances when VSM_RTX_VERIFY + dual-blur
// are both on -- once for VSM's own shadow-map search, once for the
// RTX-verified distance -- instead of collapsing them into one blended
// world_delta before blurring a single time (see CS_SHADOW_BLUR's own
// VSM_RTX_VERIFY branch for how the two resulting shadow values combine).
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

		float scaler = 1;
		float depth_gap_world = abs(pos_l_z * scaler - sampled) * depth_range;
		float w = saturate(depth_gap_world / max(penumbra_world, 0.00001));
		sum_w  += w;
		shadow += w * (sampled < pos_l_z * scaler);
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

// shadow-blur: the expensive PSO, indirect over search_tiles only. Reads
// stage 2's raw blocker-search result and turns it into the final blurred
// shadow scalar. Still has to redo the receiver's own level/slot/pos_l
// resolution (stage 1/stage 2 both already did this too, for their own
// purposes -- VSM_BlockerSearchResult only carries the WINNING TAP's data,
// not the receiver's own page) -- a 3rd resolution, same cost class
// get_shadow_vsm always paid before Phase 5.18 Part A existed, not a
// regression from this refactor.
[numthreads(16, 16, 1)]
void CS_SHADOW_BLUR(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);
	uint2 pixel = resolve_pixel(groupID, groupThreadID);
	if (any(pixel >= dims))
		return;

	VSMLighting lighting = GetVSMLighting();
	VSMConstants c = GetVSMConstants();

	float2 tc = (float2(pixel) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0)
	{
		// A search tile can still contain a sky pixel at its edge (the tile
		// was appended because SOME other pixel in it was ambiguous, not
		// because every pixel has geometry) -- sky is always lit.
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 1.0;
		return;
	}

	Camera camera = GetFrameInfo().GetCamera();
	float3 wpos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
	float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);

	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level = get_vsm_level(c, pos_ls);
	if (level < 0)
	{
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 1.0;
		return;
	}

	int resolved_level = level;
	uint slot = get_vsm_slot(c, lighting, pos_ls, level, resolved_level);
	if (slot == VSM_INVALID_SLOT)
	{
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 1.0;
		return;
	}
	level = resolved_level;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));
	float2 light_tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	if (pos_l.z < 0 || pos_l.z > 1 || any(light_tc < 0) || any(light_tc > 1))
	{
		GetVSMShadowResolveIO().GetShadow_result()[pixel] = 1.0;
		return;
	}

	float texel_world_size = c.GetLevel_info(level).z / c.GetPage_size();

	float4 vsm_depth_range_p0 = mul(page_cam.GetInvProj(), float4(0, 0, 0, 1));
	float4 vsm_depth_range_p1 = mul(page_cam.GetInvProj(), float4(0, 0, 1, 1));
	float depth_range = abs(vsm_depth_range_p1.z / vsm_depth_range_p1.w - vsm_depth_range_p0.z / vsm_depth_range_p0.w);

	float noise_angle = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).x * 6.28318530718;

	// Stage 2's packed result -- same sentinel scheme vsm_search_blocker
	// has always used (see VSM_impl_search.hlsl's own comment): most pixels
	// in a search tile are still individually confident (that's not what
	// forced the tile into search_tiles -- some OTHER pixel in the same
	// tile was ambiguous), so the sentinel branches below fire constantly,
	// not just at tile edges.
	uint4 blocker_packed = GetVSMShadowResolveIO().GetBlocker_search_result()[pixel];
	float world_delta_or_sentinel = asfloat(blocker_packed.x);

	float shadow;
	// [branch]: the final else below is the expensive path (an RTX trace
	// plus up to two 16-tap vsm_pcf_shadow calls) -- the whole point of the
	// sentinel buckets above it is to skip that work entirely for
	// individually-confident pixels. Without an explicit hint the compiler
	// is free to flatten this (compute the expensive branch for every
	// thread regardless of which case it needs, then select), which would
	// silently defeat the sentinel shortcut.
	[branch]
	if (world_delta_or_sentinel <= -4.5 && world_delta_or_sentinel > -5.5)
		shadow = 0.0; // confident_dark via a coarser level.
	else if (world_delta_or_sentinel <= -3.5 && world_delta_or_sentinel > -4.5)
		shadow = 1.0; // confident_lit via a coarser level.
	else if (world_delta_or_sentinel <= -2.5 && world_delta_or_sentinel > -3.5)
		shadow = 1.0; // confident_lit sentinel.
	else if (world_delta_or_sentinel <= -1.5)
		shadow = 0.0; // confident_dark sentinel.
	else if (world_delta_or_sentinel < 0)
		shadow = 1.0; // no blocker found.
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
		// verification ray directly at that specific point.
		static const float VSM_RTX_VERIFY_MARGIN = 1.5;
		bool  rtx_hit = false;
		float rtx_world_delta = world_delta;
		{
			Camera blocker_page_cam = lighting.GetPage_cameras()[best_slot];
			float best_sampled_z = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(best_tc, (float)best_slot), 0);
			float4 blocker_ndc  = float4(best_tc.x * 2 - 1, 1 - best_tc.y * 2, best_sampled_z, 1);
			float4 blocker_clip = mul(blocker_page_cam.GetInvViewProj(), blocker_ndc);
			float3 blocker_wpos = blocker_clip.xyz / blocker_clip.w;

			float3 sun_dir  = normalize(GetFrameInfo().GetSunDir().xyz);
			float3 to_target = blocker_wpos - wpos;
			float  target_dist = length(to_target);
			float3 aim_dir = target_dist > 0.0001 ? (to_target / target_dist) : sun_dir;

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
			ray.TMin      = 0.001;
			ray.TMax      = max(target_dist * VSM_RTX_VERIFY_MARGIN, 0.02);

			// Deliberately NOT RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH --
			// that flag returns whatever the BVH happens to traverse
			// first, not the closest hit, which would make the result an
			// unreliable distance. This needs the genuinely closest hit
			// within [TMin, TMax], so RayQuery is left to run its normal
			// closest-hit tracking to completion.
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
			// Two independent blurs, not a blended world_delta -- see
			// vsm_pcf_shadow's own comment for why min() beats a blend.
			float shadow_vsm = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, world_delta);
			float shadow_rtx = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, rtx_world_delta);
			shadow = min(shadow_vsm, shadow_rtx);
		}
		else
		{
			float chosen_delta = rtx_hit ? rtx_world_delta : world_delta;
			shadow = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, chosen_delta);
		}
#else
		shadow = vsm_pcf_shadow(c, lighting, level, pos_ls, pos_l.z, texel_world_size, depth_range, noise_angle, world_delta);
#endif
	}

	GetVSMShadowResolveIO().GetShadow_result()[pixel] = shadow;
}
