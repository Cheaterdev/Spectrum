#include "VSM_impl.hlsl"

// Root (not tables/) variant -- vsm_classify_blocker's Hi-Z classification
// (Phase 5.18 Part A) reads this via the global GetVSMPageHiZ() accessor
// directly, the same way mesh_shader_vsm.hlsl's occlusion test does, rather
// than threading it through as an explicit function parameter like
// VSMConstants/VSMLighting. Only this file needs it (get_shadow_vsm/
// vsm_pcf_shadow in VSM_impl_resolve.hlsl never touch the pyramid), which is
// the whole point of the split -- editing VSMPageHiZ.h shouldn't force
// VSMApplyCompute (VSM.hlsl) to recompile.
#include "autogen/VSMPageHiZ.h"

// Phase 5.18 Part A follow-up (take 4): everything vsm_search_blocker needs
// UP THROUGH the cheap Hi-Z classify step, factored into its own function so
// VSM_BlockerClassify.hlsl's stage-1 tile classification can call the exact
// same, already-debugged logic instead of forking a second copy. Returns
// enough state (level/slot/pos_l/texel_world_size/depth_range/search
// radius) for vsm_search_blocker to continue straight into its own tap loop
// without re-resolving any of it.
struct VSMBlockerClassifyResult
{
	bool  valid;
	bool  confident_lit;
	bool  confident_dark;
	// Debug view (VSM.ixx's vsm_debug_view == HizClassify): distinguishes
	// "classified directly at the receiver's own level" from "had to walk
	// to a coarser level to find a page big enough to fully contain the
	// search disc" -- see get_shadow_vsm's own bucket comment.
	bool  via_coarser;
	int   level;
	uint  slot;
	float2 pos_ls;
	float4 pos_l;
	float texel_world_size;
	float depth_range;
	float blocker_search_radius_texels;
	// Light-camera world-space basis (xaxis/yaxis = the tangent plane
	// pos_ls.x/.y are measured along; zaxis = page_cam.GetDirection(), the
	// light's own forward/into-scene axis) -- see vsm_search_blocker's own
	// hemisphere-cull comment for why/how these get used. Computed once
	// here (from page_cam.GetDirection() + the exact same up-vector
	// VSM.cpp's make_light_view_camera() uses, replicating look_at()'s own
	// xaxis/yaxis construction bit-for-bit) rather than trying to read them
	// back out of c.GetLight_view()'s matrix, which would need this file to
	// commit to that matrix's row/column storage convention.
	float3 xaxis;
	float3 yaxis;
	float3 zaxis;
};

// Callers must NOT early-return before calling this for any reason (sky
// pixels, screen-edge padding threads, etc.) when the caller itself goes on
// to do quad-shared work (vsm_search_blocker's search loop) -- see that
// function's own comment. This function itself does no quad ops.
VSMBlockerClassifyResult vsm_classify_blocker(VSMConstants c, VSMLighting lighting, float3 wpos,
                                               // Receiver's own surface normal and the (toward-
                                               // light) sun direction -- both already computed by
                                               // every caller for its own geometric_dark check
                                               // below, threaded through here (rather than
                                               // re-derived) purely for vsm_depth_bias_ndc's bias
                                               // (see its own comment) once pos_l is known.
                                               float3 normal, float3 light_dir,
                                               // True when the receiver's own surface normal
                                               // faces away from the light (NdotL <= 0) --
                                               // combine_result's final NL*shadow multiply
                                               // already zeroes these pixels out regardless of
                                               // the shadow value, so there's nothing here worth
                                               // spending a Hi-Z sample or a 16-tap search on.
                                               // Folded into confident_dark below (an exact
                                               // result, not an approximation).
                                               bool geometric_dark)
{
	VSMBlockerClassifyResult result = (VSMBlockerClassifyResult)0;

	float2 pos_ls = mul(c.GetLight_view(), float4(wpos, 1)).xy;
	int level_raw = get_vsm_level(c, pos_ls);
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

	// See VSMBlockerClassifyResult's own comment. up_hint is the exact
	// literal VSM.cpp's make_light_view_camera() uses, not a generic axis
	// guess -- must match so xaxis/yaxis reconstruct the SAME basis
	// c.GetLight_view() was actually built from.
	float3 zaxis = page_cam.GetDirection().xyz;
	float3 up_hint = float3(0.01, 1, 0.023);
	float3 xaxis = normalize(cross(up_hint, zaxis));
	float3 yaxis = cross(zaxis, xaxis);

	static const float VSM_BLOCKER_SEARCH_RADIUS_WORLD = 3.0;
	float texel_world_size = c.GetLevel_info(level).z / c.GetPage_size();

	float4 vsm_depth_range_p0 = mul(page_cam.GetInvProj(), float4(0, 0, 0, 1));
	float4 vsm_depth_range_p1 = mul(page_cam.GetInvProj(), float4(0, 0, 1, 1));
	float depth_range = abs(vsm_depth_range_p1.z / vsm_depth_range_p1.w - vsm_depth_range_p0.z / vsm_depth_range_p0.w);

	// See vsm_depth_bias_ndc's own comment (VSM_impl.hlsl) -- applied here
	// (not just in the final PCF resolve) so the blocker search's own
	// `sampled > pos_l.z` test and the world_delta it hands to stage 3 are
	// already consistent with the biased receiver, not the raw one.
	pos_l.z = saturate(pos_l.z - vsm_depth_bias_ndc(normal, light_dir, texel_world_size, depth_range));

	float blocker_search_radius_texels = clamp(
		VSM_BLOCKER_SEARCH_RADIUS_WORLD / texel_world_size, 2.0, c.GetPage_size() * 4.0);

	// Phase 5.18 Part A (follow-up: single-pyramid closed-form level pick,
	// replacing the earlier page_hiz/level_hiz two-tier fallback): cheap
	// classification against a page_hiz pyramid before paying for the
	// 16-tap search below. One SampleLevel at the mip whose footprint
	// covers the search disc: .x = MIN/farthest-from-light, .y =
	// MAX/closest-to-light over that footprint (see VSMPageHiZ's own
	// comment).
	//
	//   .y still farther from light than the receiver -- NOTHING in the
	//   whole search disc can possibly be a blocker. confident_lit.
	//   .x still closer to light than the receiver -- EVERYTHING in the
	//   disc blocks. confident_dark.
	//
	// Which LEVEL's page_hiz to sample: page_world_size doubles every
	// level (VSMClipmap's geometric ladder), so footprint(level, mip) =
	// texel_world_size(level) * 2^mip depends only on (level + mip)
	// combined -- one level up is interchangeable with one mip down for
	// the same footprint. That means for the fixed search diameter below,
	// there's always some level -- at or coarser than the receiver's own
	// -- whose OWN page is already bigger than the whole disc, so its
	// page_hiz (built every frame for every resident page regardless of
	// dirty state) can answer with a single corner-sampled fetch, no
	// second per-level aggregate pyramid needed. Walking coarser only
	// ever makes the classification MORE conservative (a bigger, still-
	// exact min/max footprint), never wrong -- it just costs a few more
	// ambiguous fallbacks to the real search in the levels where it
	// wasn't actually necessary.
	static const float VSM_BLOCKER_SEARCH_DIAMETER_WORLD = VSM_BLOCKER_SEARCH_RADIUS_WORLD * 2.0;

	bool confident_lit = false;
	// Geometric self-shadow (see this function's own geometric_dark
	// parameter comment) folds straight into confident_dark, reusing the
	// existing sentinel/debug color rather than a new one -- functionally
	// identical to a Hi-Z-proven confident_dark (shadow=0, skip the PCF
	// pass), just arrived at for a different, always-exact reason.
	bool confident_dark = geometric_dark;
	bool via_coarser = false;
	if (valid && !geometric_dark && c.GetHiz_blocker_classify() != 0)
	{
		float page_world_size_here = c.GetLevel_info(level).z;
		int   level_step = max(0, (int)ceil(log2(max(VSM_BLOCKER_SEARCH_DIAMETER_WORLD / max(page_world_size_here, 0.0001), 1.0))));
		int   classify_level = min(level + level_step, c.GetActive_max());
		via_coarser = classify_level > level;

		float classify_texel_world_size = c.GetLevel_info(classify_level).z / c.GetPage_size();
		float classify_radius_texels = clamp(
			VSM_BLOCKER_SEARCH_RADIUS_WORLD / classify_texel_world_size, 2.0, c.GetPage_size() * 4.0);
		float classify_world_radius = classify_radius_texels * classify_texel_world_size;

		// Resolve each of the 4 corners of the search rect to ITS OWN page
		// independently (via vsm_resolve_tap, the exact same per-tap page
		// lookup the real search loop uses) instead of requiring the whole
		// rect to fit inside one page. level_step above only fixes the
		// DISC-SIZE-vs-PAGE-SIZE ratio -- it says nothing about whether the
		// receiver happens to sit near a page BOUNDARY, which can occur at
		// any level (page boundaries are independently snapped per level,
		// so a position can sit near an edge at several levels in a row).
		// Requiring single-page containment turned that into permanent
		// classification holes running along every page seam, confirmed
		// live. Resolving corners independently removes the restriction
		// entirely: a rect straddling 2-4 neighboring pages just samples
		// from 2-4 different slots, same tolerance the real per-tap search
		// already has for exactly this reason.
		static const float2 CORNER_SIGNS[4] = { float2(-1, -1), float2(1, -1), float2(-1, 1), float2(1, 1) };
		uint   corner_slots[4];
		float2 corner_uvs[4];
		bool   all_resolved = true;
		[unroll]
		for (int ci = 0; ci < 4; ci++)
		{
			float2 corner_pos_ls = pos_ls + CORNER_SIGNS[ci] * classify_world_radius;
			if (!vsm_resolve_tap(c, lighting, classify_level, corner_pos_ls, corner_slots[ci], corner_uvs[ci]))
				all_resolved = false;
		}

		if (all_resolved)
		{
			Texture2DArray<float2> pyramid = GetVSMPageHiZ().GetPage_hiz();
			uint pw, ph, elems, numLevels;
			pyramid.GetDimensions(0, pw, ph, elems, numLevels);

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
			// with no gap regardless of alignment. Computed once against
			// classify_level's own texel density -- still valid (only ever
			// MORE conservative, never too fine) for a corner that
			// vsm_resolve_tap had to resolve onto an even coarser page,
			// since numLevels (the pyramid's mip depth) is the same fixed
			// constant for every page regardless of level.
			float mip_f = ceil(log2(max(classify_radius_texels * 2.0, 1.0)));

			// Clamping mip_f down to numLevels-1 when the pyramid isn't
			// deep enough does NOT give a conservative coverage failure
			// -- it silently reintroduces the same coverage-gap bug
			// fixed above, one level up. Only proceed when the pyramid
			// genuinely has enough depth to cover this radius safely;
			// otherwise leave both flags false (ambiguous).
			bool mip_available = mip_f <= (float)(numLevels - 1);

			if (mip_available)
			{
				uint mip = (uint)mip_f;

				float2 minmax = float2(3.402823466e+38, -3.402823466e+38);
				float corner_x[4];
				[unroll]
				for (int cj = 0; cj < 4; cj++)
				{
					float2 s = pyramid.SampleLevel(pointClampSampler, float3(corner_uvs[cj], (float)corner_slots[cj]), mip);
					minmax.x = min(minmax.x, s.x);
					minmax.y = max(minmax.y, s.y);
					corner_x[cj] = s.x;
				}

				// pos_l.z (the RECEIVER's own NDC-Z, from its own
				// render-level page camera) stays valid to compare against
				// every corner's own resolved page even though each can
				// belong to a different physical slot -- every page camera
				// shares the identical fixed Z near/far range
				// (VSM_LIGHT_Z_NEAR/FAR), so NDC-Z is on the same scale
				// everywhere; only XY differs per page.
				confident_lit  = minmax.y < pos_l.z;
				confident_dark = !confident_lit && minmax.x > pos_l.z;

				// Hemisphere check (see use_vsm_hemisphere_cull's own
				// comment, VSM.ixx, and vsm_search_blocker's matching
				// per-tap version below): confident_dark claims EVERY point
				// in the whole search disc occludes the receiver, which
				// (per the math above) implicitly requires each corner's
				// own worst-case value to individually clear the same depth
				// test too. A corner whose worst case actually sits BEHIND
				// the receiver's tangent plane (the near-two-sided-floor
				// case this toggle exists for) isn't a valid occluder at
				// all -- its depth passing the test proves nothing. Only
				// ever narrows confident_dark to "fall through to the real
				// search" (never flips a real dark verdict to lit outright);
				// the search below does its own per-tap hemisphere cull and
				// can still legitimately find a genuine blocker.
				if (confident_dark && c.GetHemisphere_cull_blocker() != 0)
				{
					[unroll]
					for (int ck = 0; ck < 4; ck++)
					{
						float2 tangent_offset = CORNER_SIGNS[ck] * classify_world_radius;
						float3 blocker_delta = xaxis * tangent_offset.x + yaxis * tangent_offset.y
						                      - zaxis * ((corner_x[ck] - pos_l.z) * depth_range);
						static const float VSM_HEMISPHERE_CULL_EPS_WORLD = 0.02;
						if (dot(blocker_delta, normal) < -VSM_HEMISPHERE_CULL_EPS_WORLD)
						{
							confident_dark = false;
							break;
						}
					}
				}
			}
		}
	}

	result.valid = valid;
	result.confident_lit = confident_lit;
	result.confident_dark = confident_dark;
	result.via_coarser = via_coarser;
	result.level = level;
	result.slot = slot;
	result.pos_ls = pos_ls;
	result.pos_l = pos_l;
	result.texel_world_size = texel_world_size;
	result.depth_range = depth_range;
	result.blocker_search_radius_texels = blocker_search_radius_texels;
	result.xaxis = xaxis;
	result.yaxis = yaxis;
	result.zaxis = zaxis;
	return result;
}

// Blocker-search extraction: everything get_shadow_vsm used to do UP
// THROUGH finding the search result now lives here, called once per pixel
// from VSM_BlockerSearch's own dispatch (VSM_BlockerSearch.hlsl) instead of
// inline inside the same dispatch as the final PCF blur/shading. Packs its
// result into a uint4 for VSMBlockerSearchOutput's blocker_search_result
// field (see that field's own comment for the exact layout) -- asuint(-1.0)
// in .x is the sentinel for "no blocker found" (world_delta is otherwise
// always >=0 by construction).
//
// Unconditional (no #ifdef guard) -- VSM.cpp only ever runs this pass when
// penumbra mode is on in the first place, via its own setup()'s early-out,
// so this function must compile standalone.
//
// Callers must NOT early-return before calling this for any reason (sky
// pixels, screen-edge padding threads, etc.) -- see `valid` below: this
// function's own quad-shared search mode needs every thread in a 2x2
// dispatch quad to reach its QuadReadAcrossX/Y/Diagonal calls uniformly, a
// requirement that propagates all the way up through this function's
// caller (VSM_BlockerSearch.hlsl's CS_BLOCKER_SEARCH). The same rule means
// `geometric_dark` (the caller's own NdotL<=0 test -- see its own parameter
// comment below) must be passed IN rather than used by the caller to skip
// calling this function outright.
uint4 vsm_search_blocker(VSMConstants c, VSMLighting lighting, float3 wpos, uint2 pixel, float3 normal, float3 light_dir, bool geometric_dark)
{
	VSMBlockerClassifyResult cls = vsm_classify_blocker(c, lighting, wpos, normal, light_dir, geometric_dark);
	bool  valid                        = cls.valid;
	int   level                        = cls.level;
	uint  slot                         = cls.slot;
	float2 pos_ls                      = cls.pos_ls;
	float4 pos_l                       = cls.pos_l;
	float texel_world_size             = cls.texel_world_size;
	float depth_range                  = cls.depth_range;
	float blocker_search_radius_texels = cls.blocker_search_radius_texels;
	bool  confident_lit                = cls.confident_lit;
	bool  confident_dark               = cls.confident_dark;
	bool  via_coarser                  = cls.via_coarser;
	float3 xaxis                       = cls.xaxis;
	float3 yaxis                       = cls.yaxis;
	float3 zaxis                       = cls.zaxis;

	bool hemisphere_cull = c.GetHemisphere_cull_blocker() != 0;

	int  search_mode = c.GetQuad_blocker_search();
	bool quad_search = search_mode == 1;

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
	int bi_start  = quad_search ? (int)quad_lane : 0;
	int bi_stride = quad_search ? 4 : 1;
	int bi_count  = quad_search ? 4 : 16;

	[loop]
	for (int bii = 0; bii < bi_count; bii++)
	{
		if (!valid || confident_lit || confident_dark)
			continue;
		int bi = bi_start + bii * bi_stride;
		float2 rotated_offset = vsm_rotate(VSM_POISSON_DISK[bi], search_noise_angle);
		uint tap_slot;
		float2 tc;
		if (!vsm_tap(c, lighting, level, pos_ls, texel_world_size,
		             rotated_offset, blocker_search_radius_texels,
		             tap_slot, tc))
			continue;
		float sampled = lighting.GetVsm_atlas().SampleLevel(pointClampSampler, float3(tc, (float)tap_slot), 0);
        if (sampled > pos_l.z)
        {
            // Hemisphere cull: reconstruct this candidate's world-space
            // offset from the receiver (same tangent_offset formula
            // vsm_tap uses internally for its light-space XY, the sampled
            // depth converted to a world-space distance along zaxis for
            // the third) and discard it if it sits behind the receiver's
            // own tangent plane -- see use_vsm_hemisphere_cull's own
            // comment (VSM.ixx) for the motivating case. A small negative
            // epsilon (not a strict >=0) tolerates the receiver's own
            // surface reappearing at glancing angles without over-culling.
            if (hemisphere_cull)
            {
                float2 tangent_offset = rotated_offset * blocker_search_radius_texels * texel_world_size * float2(1, -1);
                float3 blocker_delta = xaxis * tangent_offset.x + yaxis * tangent_offset.y
                                      - zaxis * ((sampled - pos_l.z) * depth_range);
                static const float VSM_HEMISPHERE_CULL_EPS_WORLD = 0.02;
                if (dot(blocker_delta, normal) < -VSM_HEMISPHERE_CULL_EPS_WORLD)
                    continue;
            }

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

		// Depth-discontinuity guard: a neighbor lane's "blocker" was only
		// ever validated against THAT lane's own receiver depth (pos_l.z),
		// not mine -- at a silhouette edge or any sharp depth change
		// across the quad, a sample that legitimately blocked a farther
		// neighbor can be closer to the light than MY OWN surface, i.e.
		// not a blocker for me at all. Trusting whichever lane found the
		// raw-largest sample (as this used to) could adopt that neighbor's
		// value anyway, producing a bogus (sometimes negative) world_delta
		// -- confirmed as the likely source of quad_search's edge-case
		// divergence from the non-quad search. Re-validate each candidate
		// against THIS thread's own pos_l.z before accepting it.
		bool valid_x = max_x > pos_l.z;
		bool valid_y = max_y > pos_l.z;
		bool valid_d = max_d > pos_l.z;

		blocker_count += (valid_x ? cnt_x : 0) + (valid_y ? cnt_y : 0) + (valid_d ? cnt_d : 0);
		if (valid_x && max_x > max_blocker_z) { max_blocker_z = max_x; best_tc = tc_x; best_slot = slot_x; best_sampled_z = z_x; }
		if (valid_y && max_y > max_blocker_z) { max_blocker_z = max_y; best_tc = tc_y; best_slot = slot_y; best_sampled_z = z_y; }
		if (valid_d && max_d > max_blocker_z) { max_blocker_z = max_d; best_tc = tc_d; best_slot = slot_d; best_sampled_z = z_d; }
	}

	// confident_lit collapses into the exact same asuint(-1.0) sentinel a
	// genuine empty search already uses, so there's no way to tell
	// "classification said lit" apart from "the real search would have
	// said lit too" downstream without a distinct sentinel -- asuint(-3.0)/
	// asuint(-4.0) (via_coarser) make it distinguishable. VSM_DebugClassifyOverlay's
	// HizClassify view colors these for visual confirmation.
	if (confident_lit)
		return uint4(asuint(via_coarser ? -4.0 : -3.0), 0, 0, 0);

	// confident_dark: the tap loop above never ran for this thread (skipped
	// via continue), so blocker_count is 0 here same as the genuine
	// no-blocker case below -- must check this FIRST, or a confidently
	// fully-shadowed pixel would wrongly fall into the "no blocker, fully
	// lit" sentinel just below. asuint(-2.0)/asuint(-5.0): distinct from
	// asuint(-1.0)'s "no blocker" -- get_shadow_vsm/vsm_pcf_shadow's caller
	// reads this back and skips the PCF pass entirely, shadow = 0.
	if (confident_dark)
		return uint4(asuint(via_coarser ? -5.0 : -2.0), 0, 0, 0);

	if (blocker_count == 0)
        return uint4(asuint(-1.0), 0, 0, 0);

	float world_delta = (max_blocker_z - pos_l.z) * depth_range;
	return uint4(asuint(world_delta), asuint(best_tc.x), asuint(best_tc.y), best_slot);
}
