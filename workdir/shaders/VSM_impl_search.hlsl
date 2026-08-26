#include "VSM_impl.hlsl"

// Root (not tables/) variant -- vsm_search_blocker's Hi-Z classification
// (Phase 5.18 Part A) reads this via the global GetVSMPageHiZ() accessor
// directly, the same way mesh_shader_vsm.hlsl's occlusion test does, rather
// than threading it through as an explicit function parameter like
// VSMConstants/VSMLighting. Only this file needs it (get_shadow_vsm/
// vsm_pcf_shadow in VSM_impl_resolve.hlsl never touch the pyramid), which is
// the whole point of the split -- editing VSMPageHiZ.h shouldn't force
// VSMApplyCompute (VSM.hlsl) to recompile.
#include "autogen/VSMPageHiZ.h"

// Blocker-search extraction: everything get_shadow_vsm used to do UP
// THROUGH finding the search result now lives here, called once per pixel
// from VSM_BlockerSearch's own dispatch (VSM_BlockerSearch.hlsl) instead of
// inline inside the same dispatch as the final PCF blur/shading. Packs its
// result into a uint4 for VSMLighting's blocker_result field (see that
// field's own comment for the exact layout) -- asuint(-1.0) in .x is the
// sentinel for "no blocker found" (world_delta is otherwise always >=0 by
// construction).
//
// Unconditional (not #ifdef VSM_PENUMBRA-guarded) -- VSMBlockerSearchCompute
// has no VsmPenumbra define at all (VSM.cpp only ever runs this pass when
// penumbra mode is on in the first place, via its own setup()'s early-out),
// so this function must compile standalone.
//
// Callers must NOT early-return before calling this for any reason (sky
// pixels, screen-edge padding threads, etc.) -- see `valid` below: this
// function's own quad-shared search mode needs every thread in a 2x2
// dispatch quad to reach its QuadReadAcrossX/Y/Diagonal calls uniformly, a
// requirement that propagates all the way up through this function's
// caller (VSM_BlockerSearch.hlsl's CS_BLOCKER_SEARCH).
uint4 vsm_search_blocker(VSMConstants c, VSMLighting lighting, float3 wpos, uint2 pixel)
{
	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level_raw = get_vsm_level(c, pos_ls);
	// See get_shadow_vsm's own history for the fuller version of this
	// early-return-avoidance reasoning -- moved here wholesale along with
	// the quad ops it exists to protect; get_shadow_vsm itself no longer
	// does any quad ops, so it went back to plain early returns.
	bool valid = level_raw >= 0;
	int level = valid ? level_raw : c.GetActive_min();

	int resolved_level = level;
	uint slot_raw = valid ? get_vsm_slot(c, lighting, pos_ls, level, resolved_level) : VSM_INVALID_SLOT;
	valid = valid && (slot_raw != VSM_INVALID_SLOT);
	uint slot = valid ? slot_raw : 0;
	// get_vsm_slot can walk out to a COARSER level than the one just
	// resolved above (residency fallback) -- from here on, `level` means
	// "the level the resolved slot actually belongs to", not "the level
	// the receiver's own position would ideally use". Using the original,
	// possibly-finer level for level_info lookups after this point mixes
	// one level's geometry with a different level's actual page -- see
	// get_vsm_slot's own comment for how this surfaced live.
	level = valid ? resolved_level : level;

	Camera page_cam = lighting.GetPage_cameras()[slot];
	float4 pos_l = mul(page_cam.GetViewProj(), float4(wpos, 1));

	static const float VSM_BLOCKER_SEARCH_RADIUS_WORLD = 4.0;
	float texel_world_size = c.GetLevel_info(level).z / c.GetPage_size();

	float4 vsm_depth_range_p0 = mul(page_cam.GetInvProj(), float4(0, 0, 0, 1));
	float4 vsm_depth_range_p1 = mul(page_cam.GetInvProj(), float4(0, 0, 1, 1));
	float depth_range = abs(vsm_depth_range_p1.z / vsm_depth_range_p1.w - vsm_depth_range_p0.z / vsm_depth_range_p0.w);

	float blocker_search_radius_texels = clamp(
		VSM_BLOCKER_SEARCH_RADIUS_WORLD / texel_world_size, 2.0, c.GetPage_size() * 4.0);

	// Phase 5.18 Part A: cheap classification against the receiver's own
	// page Hi-Z pyramid (VSMPageHiZ, rebuilt fresh this frame by
	// VSM_HiZRebuild -- see that pass's comment for the ordering this
	// depends on) before paying for the 16-tap search below. One
	// SampleLevel at the mip whose footprint covers the search disc: .x =
	// MIN/farthest-from-light, .y = MAX/closest-to-light over that
	// footprint (see VSMPageHiZ's own comment).
	//
	//   .y still farther from light than the receiver -- NOTHING in the
	//   whole search disc can possibly be a blocker. confident_lit.
	//   .x still closer to light than the receiver -- EVERYTHING in the
	//   disc blocks. confident_dark.
	//
	// Must NOT early-`return` for either case -- see this function's own
	// top comment: quad_search's QuadReadAcrossX/Y/Diagonal calls further
	// down need every thread in a dispatch quad to reach them, and a
	// per-pixel classification differs lane-to-lane within a quad even
	// though quad_search itself (a uniform constant) does not. Instead,
	// only the per-tap loop's CONTRIBUTION is skipped below (same shape as
	// the existing `!valid` skip); the quad-merge code, the confident_dark
	// check, and the final return all still run for every thread, exactly
	// where they already did.
	bool confident_lit = false;
	bool confident_dark = false;
	// TEMP DEBUG (live "does level_hiz fallback even fire" investigation):
	// tracks which pyramid actually produced the classification, so the
	// debug view can color them differently. Remove once confirmed.
	bool via_level = false;
	if (valid && c.GetHiz_blocker_classify() != 0)
	{
		Texture2DArray<float2> pyramid = GetVSMPageHiZ().GetPage_hiz();
		uint pw, ph, elems, numLevels;
		pyramid.GetDimensions(0, pw, ph, elems, numLevels);

		float2 tc = pos_l.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
		// The search disc's bounding rect in UV space -- radius in texels
		// converted to UV via page size, same radius the tap loop below
		// actually searches.
		float2 rect_uv = blocker_search_radius_texels / float2(pw, ph);
		float2 uv_min = tc - rect_uv;
		float2 uv_max = tc + rect_uv;

		// VSMPageHiZ is one pyramid PER PAGE (isolated per physical atlas
		// slot by design -- mip downsampling must not bleed across
		// unrelated tiles, see VSMPageHiZ's own comment), so it only ever
		// knows about the receiver's own page. tc is this page's own
		// camera's NDC->UV, so outside [0,1] genuinely means "past this
		// page's edge" -- the real tap loop below can and does resolve
		// such a tap into a DIFFERENT physical page via vsm_resolve_tap's
		// own independent page-table walk, but this page's pyramid can't
		// see that neighbor's content at all. Sampling it anyway just
		// reads this page's own edge-clamped value repeated outward,
		// which can silently disagree with what's actually resident next
		// door -- classifying confidently based on data that was never
		// really there. Only classify when the WHOLE rect stays inside
		// this page; otherwise leave both flags false (ambiguous) and let
		// the real per-tap search -- which resolves neighbors correctly --
		// handle it, same as it always did before this optimization.
		bool rect_in_page = all(uv_min >= 0) && all(uv_max <= 1);
		bool classified = false;

		if (rect_in_page)
		{
			// Mip pick, deliberately NOT mesh_shader_vsm.hlsl's own
			// "<=2x2 texels, *0.5" formula -- that bound has a real gap: an
			// interval of width exactly 2 texels can still touch THREE
			// texel indices in the worst alignment (e.g. [0.5, 2.5] touches
			// 0, 1, 2), so sampling only the two corner texels can miss a
			// real blocker sitting in the untouched middle one. Harmless
			// for occlusion culling (a miss there just means an object
			// isn't culled -- a perf cost, not a correctness bug) but not
			// for this: a missed blocker here means a wrongly confident_lit
			// classification -- a shadow that should be there vanishing
			// entirely, confirmed live. Dropping the "*0.5" picks the mip
			// where the rect spans at most ONE texel instead, which bounds
			// the worst-case span to exactly two texel indices per axis --
			// genuinely fully covered by the four corner samples below,
			// with no gap regardless of alignment.
			float2 rect_texels = (uv_max - uv_min) * float2(pw, ph);
			float  mip_f = ceil(log2(max(max(rect_texels.x, rect_texels.y), 1.0)));

			// pyramid_mip_count is a fixed, level-independent constant
			// (derived once from page_size, same for every page). At fine
			// clip levels close to the camera, texel_world_size shrinks, so
			// blocker_search_radius_texels (a roughly fixed WORLD-space
			// radius) balloons in texel terms -- routinely needing a mip
			// deeper than the pyramid actually has. Clamping mip_f down to
			// numLevels-1 in that case does NOT give a conservative
			// coverage failure -- it silently reuses the SAME coverage-gap
			// bug just fixed above, one level up: the clamped mip is finer
			// than the guarantee requires, so the 4 corners can once again
			// miss a real blocker sitting between them. Confirmed live on
			// close-up, fine-level geometry -- a shadow that should be
			// there vanishing entirely. Only proceed when the pyramid
			// genuinely has enough depth to cover this radius safely;
			// otherwise leave both flags false (ambiguous), same fallback
			// as the page-edge case above.
			bool mip_available = mip_f <= (float)(numLevels - 1);

			if (mip_available)
			{
				uint mip = (uint)mip_f;

				float2 c00 = pyramid.SampleLevel(pointClampSampler, float3(uv_min.x, uv_min.y, (float)slot), mip);
				float2 c10 = pyramid.SampleLevel(pointClampSampler, float3(uv_max.x, uv_min.y, (float)slot), mip);
				float2 c01 = pyramid.SampleLevel(pointClampSampler, float3(uv_min.x, uv_max.y, (float)slot), mip);
				float2 c11 = pyramid.SampleLevel(pointClampSampler, float3(uv_max.x, uv_max.y, (float)slot), mip);
				float2 minmax = float2(min(min(c00.x, c10.x), min(c01.x, c11.x)),
				                       max(max(c00.y, c10.y), max(c01.y, c11.y)));

				confident_lit  = minmax.y < pos_l.z;
				confident_dark = !confident_lit && minmax.x > pos_l.z;
				classified = true;
			}
		}

		// Fallback (Phase 5.18 follow-up multi-page pyramid): the disc
		// didn't fit in the receiver's own page, or needed a mip deeper
		// than page_hiz has -- try VSMPageHiZ's level_hiz field (a much
		// smaller pyramid, one texel per PAGE, spanning the whole level's
		// page grid) instead of giving up outright. Same shape as the
		// page-local query above, just in LEVEL-GRID space: light-space XY
		// directly (no page-camera-NDC Y-flip -- level_hiz's texels are
		// indexed by raw page_x/page_y, the same convention get_vsm_level's
		// own extent/rel math already uses, not the page-camera convention
		// `tc` above uses), queried against level_hiz instead of page_hiz.
		// Comparable against the SAME pos_l.z: every page's own camera
		// within a level shares the identical Z near/far range (VSM.cpp's
		// page_cam setup derives it from the per-LEVEL plan, not per-page),
		// so NDC-Z values are on the same scale across any page of this
		// level -- the real per-tap search already relies on this exact
		// same cross-page comparability via vsm_resolve_tap's own
		// cross-page walk, so this isn't a new assumption.
		if (!classified)
		{
			Texture2DArray<float2> lpyramid = GetVSMPageHiZ().GetLevel_hiz();
			uint lw, lh, lelems, lnumLevels;
			lpyramid.GetDimensions(0, lw, lh, lelems, lnumLevels);

			float4 linfo = c.GetLevel_info(level);
			float  level_extent = linfo.z * c.GetPages_per_level();
			float2 level_tc = (pos_ls - linfo.xy) / max(level_extent, 0.0001);
			// texel_world_size/level_extent cancels down to this, same
			// simplification reasoning as the page-local rect_uv above.
			float lrect_uv = blocker_search_radius_texels / (c.GetPage_size() * c.GetPages_per_level());
			float2 luv_min = level_tc - lrect_uv;
			float2 luv_max = level_tc + lrect_uv;

			bool lrect_in_level = all(luv_min >= 0) && all(luv_max <= 1);
			if (lrect_in_level)
			{
				float2 lrect_texels = (luv_max - luv_min) * float2(lw, lh);
				float  lmip_f = ceil(log2(max(max(lrect_texels.x, lrect_texels.y), 1.0)));
				// lnumLevels - 2, NOT - 1: the coarsest mip (1x1, e.g. at
				// pages_per_level=4 that's ALL 16 pages of the level
				// collapsed into one min/max pair) technically satisfies
				// the coverage guarantee, but "everything closer to light
				// ANYWHERE in the whole level" is not a spatially
				// meaningful bound for a single receiver -- a level can
				// span the entire visible scene, so whichever one page
				// happens to contain the tallest nearby building dominates
				// the reduction for every other receiver in the level,
				// producing a classification that reads as flatly wrong
				// (not just imprecise) wherever it wins the confidence
				// check. Confirmed live: rectangular patches of confident_
				// lit/dark appearing with no correlation to real geometry.
				// Capping one mip short keeps the fallback to at most a
				// 2x2-page neighborhood -- still meaningfully local.
				bool   lmip_available = lmip_f <= (float)(lnumLevels - 2);

				if (lmip_available)
				{
					uint lmip = (uint)lmip_f;
					float2 lc00 = lpyramid.SampleLevel(pointClampSampler, float3(luv_min.x, luv_min.y, (float)level), lmip);
					float2 lc10 = lpyramid.SampleLevel(pointClampSampler, float3(luv_max.x, luv_min.y, (float)level), lmip);
					float2 lc01 = lpyramid.SampleLevel(pointClampSampler, float3(luv_min.x, luv_max.y, (float)level), lmip);
					float2 lc11 = lpyramid.SampleLevel(pointClampSampler, float3(luv_max.x, luv_max.y, (float)level), lmip);
					float2 lminmax = float2(min(min(lc00.x, lc10.x), min(lc01.x, lc11.x)),
					                        max(max(lc00.y, lc10.y), max(lc01.y, lc11.y)));

					confident_lit  = lminmax.y < pos_l.z;
					confident_dark = !confident_lit && lminmax.x > pos_l.z;
					via_level      = true;
				}
			}
		}
	}

	int  search_mode       = c.GetQuad_blocker_search();
	bool quad_search        = search_mode == 1;
	bool stochastic_search  = search_mode == 2;

	uint2 search_noise_pixel = quad_search ? (pixel & ~1u) : pixel;
	float search_noise_angle = lighting.GetBlue_noise().Load(int3(search_noise_pixel % 128, 0)).x * 6.28318530718;

	float max_blocker_z = 0;
	float best_sampled_z = 0;
	float2 best_tc = 0;
	uint best_slot = 0;
	float best_pick_noise = lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).y;
	static const float VSM_RTX_TAP_TIE_EPS_WORLD = 0.1;
	int blocker_count = 0;

	uint quad_lane = (pixel.x & 1) + (pixel.y & 1) * 2;
	int random_tap = (int)(frac(lighting.GetBlue_noise().Load(int3(pixel % 128, 0)).x * 4327.31) * 16.0);
	random_tap = clamp(random_tap, 0, 15);
	int bi_start  = stochastic_search ? random_tap : (quad_search ? (int)quad_lane : 0);
	int bi_stride = quad_search ? 4 : 1;
	int bi_count  = stochastic_search ? 1 : (quad_search ? 4 : 16);

	[loop]
	for (int bii = 0; bii < bi_count; bii++)
	{
		if (!valid || confident_lit || confident_dark)
			continue;
		int bi = bi_start + bii * bi_stride;
		uint tap_slot;
		float2 tc;
		if (!vsm_tap(c, lighting, level, pos_ls, texel_world_size,
		             vsm_rotate(VSM_POISSON_DISK[bi], search_noise_angle), blocker_search_radius_texels,
		             tap_slot, tc))
			continue;
		float sampled = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(tc, (float)tap_slot), 0);
        if (sampled > pos_l.z)
        {
            bool is_new_max = sampled > max_blocker_z || blocker_count == 0;
            bool is_near_tie = !is_new_max
				&& (max_blocker_z - sampled) * depth_range < VSM_RTX_TAP_TIE_EPS_WORLD
				&& best_pick_noise > 0.5;
            if (is_new_max || is_near_tie)
            {
                best_sampled_z = sampled;
                best_tc = tc;
                best_slot = tap_slot;
            }
            max_blocker_z = max(max_blocker_z, sampled);
            blocker_count++;
        }
    }

	if (quad_search)
	{
		float  max_x = QuadReadAcrossX(max_blocker_z);
		float  max_y = QuadReadAcrossY(max_blocker_z);
		float  max_d = QuadReadAcrossDiagonal(max_blocker_z);
		uint   cnt_x = QuadReadAcrossX(blocker_count);
		uint   cnt_y = QuadReadAcrossY(blocker_count);
		uint   cnt_d = QuadReadAcrossDiagonal(blocker_count);
		float2 tc_x  = QuadReadAcrossX(best_tc);
		float2 tc_y  = QuadReadAcrossY(best_tc);
		float2 tc_d  = QuadReadAcrossDiagonal(best_tc);
		uint   slot_x = QuadReadAcrossX(best_slot);
		uint   slot_y = QuadReadAcrossY(best_slot);
		uint   slot_d = QuadReadAcrossDiagonal(best_slot);
		float  z_x   = QuadReadAcrossX(best_sampled_z);
		float  z_y   = QuadReadAcrossY(best_sampled_z);
		float  z_d   = QuadReadAcrossDiagonal(best_sampled_z);

		blocker_count += cnt_x + cnt_y + cnt_d;
		if (max_x > max_blocker_z) { max_blocker_z = max_x; best_tc = tc_x; best_slot = slot_x; best_sampled_z = z_x; }
		if (max_y > max_blocker_z) { max_blocker_z = max_y; best_tc = tc_y; best_slot = slot_y; best_sampled_z = z_y; }
		if (max_d > max_blocker_z) { max_blocker_z = max_d; best_tc = tc_d; best_slot = slot_d; best_sampled_z = z_d; }
	}

	// TEMP DEBUG (live "shadow block vanishes" investigation): confident_lit
	// collapses into the exact same asuint(-1.0) sentinel a genuine empty
	// search already uses, so there's no way to tell "classification said
	// lit" apart from "the real search would have said lit too" downstream
	// -- asuint(-3.0) makes it distinguishable. get_shadow_vsm's
	// VSM_DEBUG_HIZ_CLASSIFY branch colors it for visual confirmation.
	// Remove both once the bug is confirmed fixed.
	//
	// TEMP DEBUG (live "does level_hiz fallback even fire" investigation):
	// -4.0/-5.0 distinguish a via_level classification from the page-local
	// one (-3.0/-2.0) so the debug view can show them as different colors.
	// Remove alongside via_level once confirmed.
	if (confident_lit)
		return uint4(asuint(via_level ? -4.0 : -3.0), 0, 0, 0);

	// confident_dark: the tap loop above never ran for this thread (skipped
	// via continue), so blocker_count is 0 here same as the genuine
	// no-blocker case below -- must check this FIRST, or a confidently
	// fully-shadowed pixel would wrongly fall into the "no blocker, fully
	// lit" sentinel just below. asuint(-2.0): a second sentinel, distinct
	// from asuint(-1.0)'s "no blocker" -- get_shadow_vsm/vsm_pcf_shadow's
	// caller reads this back and skips the PCF pass entirely, shadow = 0.
	if (confident_dark)
		return uint4(asuint(via_level ? -5.0 : -2.0), 0, 0, 0);

	if (blocker_count == 0)
        return uint4(asuint(-1.0), 0, 0, 0);

	float world_delta = (max_blocker_z - pos_l.z) * depth_range;
	return uint4(asuint(world_delta), asuint(best_tc.x), asuint(best_tc.y), best_slot);
}
