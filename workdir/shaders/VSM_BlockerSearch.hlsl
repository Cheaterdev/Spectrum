#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMTileListRead.h"
#include "autogen/VSMBlockerSearchOutput.h"
#include "autogen/VSMSearchVerdictAppend.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

// Only the search half of VSM_impl.hlsl's old contents -- see
// VSM_impl_search.hlsl's own top comment for why this is split from
// VSM_impl_resolve.hlsl (VSM.hlsl's own include).
#include "VSM_impl_search.hlsl"

// Stage 2 (Phase 5.18 Part A follow-up, take 4): INDIRECTLY dispatched over
// only VSM_SearchTiles -- the tiles stage 1 (VSM_BlockerClassify) bucketed
// as genuinely needing real work. SV_GroupID.x indexes the list directly --
// VSMBlockerClassifyInitDispatch sized the indirect dispatch to exactly the
// appended tile count, one 16x16 group per tile. Writes the raw
// blocker-search result to VSMBlockerSearchOutput's own dedicated texture
// (NOT a texture VSM_Combine samples directly any more) for stage 3's
// shadow-blur PSO to read back -- see that struct's own comment in vsm.sig.
//
// Still does its own per-pixel classification internally
// (vsm_search_blocker calls vsm_classify_blocker itself) even though stage
// 1 already classified every pixel once -- a tile can land in
// VSM_SearchTiles because it has BOTH confidently-lit AND confidently-dark
// pixels with zero truly-ambiguous ones (a hard boundary with no penumbra
// pixel in this tile), so most of a "search" tile's pixels can still
// resolve via the cheap confident_lit/confident_dark paths inside
// vsm_search_blocker and skip the real tap loop -- only genuinely ambiguous
// pixels pay for it.
//
// Mirrors VSM.hlsl's combine_result/CS_RESULT split for the SAME reason:
// vsm_search_blocker's quad-shared search mode (VSM_impl.hlsl,
// quad_blocker_search == 1) needs every thread in a 2x2 dispatch quad to
// reach its QuadReadAcrossX/Y/Diagonal calls uniformly -- a compute-shader
// `return` genuinely deactivates a thread (no pixel-shader helper-invocation
// guarantee), so an early return for sky pixels or screen-edge padding
// threads would leave still-active quad-mates' reads undefined. Dispatching
// per whole 2x2-quad-aligned tile (16 is a multiple of 2) keeps that
// invariant intact under the indirect dispatch -- a quad never spans a tile
// boundary.
//
// Post-search verdict (see VSMSearchVerdictAppend's own comment): even a
// search_tiles tile can turn out, after every pixel's REAL result is known,
// to need no further work -- appended to confirmed_lit_tiles so stage 3 can
// skip the expensive shadow-blur PSO for it too, same as stage 1 already
// skips it for lit_tiles/dark_tiles. NOT simply "no pixel found a real
// blocker" -- a tile can have a mix of confidently-lit and confidently-dark
// pixels with zero real blockers (the hard-boundary-no-penumbra case that
// put it in search_tiles to begin with), and flat-filling that as lit would
// silently wipe its dark pixels. The reduction below tracks "any pixel
// resolved dark" and "any pixel found a real blocker" separately; only when
// NEITHER happened is confirmed_lit_tiles safe.
groupshared uint g_any_dark;
groupshared uint g_any_real_blocker;

[numthreads(16, 16, 1)]
void CS_BLOCKER_SEARCH(
	uint3 groupID       : SV_GroupID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex    : SV_GroupIndex
)
{
	if (groupIndex == 0)
	{
		g_any_dark = 0;
		g_any_real_blocker = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

	// groupID is 1D here (this is an INDIRECT dispatch, ThreadGroupCountX =
	// appended tile count, Y=Z=1) -- groupID.x is the dispatch INDEX, not a
	// screen-tile coordinate. The real coordinate is this list lookup;
	// stash it so thread 0 can append the right value below, not groupID.xy
	// itself (which stage 1's own REGULAR 2D dispatch can use directly, but
	// this indirect one can't).
	uint2 tile_coord   = GetVSMTileListRead().GetTiles()[groupID.x];
	uint2 tile_origin  = tile_coord * 16;
	uint2 pixel        = tile_origin + groupThreadID.xy;
	bool  in_bounds    = all(pixel < dims);

	float2 tc = (float2(pixel) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	bool has_geometry = (raw_z != 0);

	Camera camera = GetFrameInfo().GetCamera();
	// Sky pixels get safe, finite dummy inputs (camera position) rather
	// than whatever depth_to_wpos(0, ...) or an unwritten gbuffer normal
	// texel would produce -- vsm_search_blocker never sees NaN/Inf-risking
	// input even though its result is discarded below.
	float3 wpos = has_geometry ? depth_to_wpos(raw_z, tc, camera.GetInvViewProj()) : camera.GetPosition();

	// combine_result multiplies the final shadow by saturate(NdotL), so a
	// surface facing away from the light already contributes zero
	// regardless of the shadow value -- pass this through to
	// vsm_search_blocker (which folds it into confident_dark) instead of
	// spending a Hi-Z sample or a 16-tap search finding that out the
	// expensive way. Sky pixels (no geometry) get false here; their result
	// is overwritten by the -1.0 sentinel below regardless.
	bool geometric_dark = false;
	if (has_geometry)
	{
		float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);
		float3 light_dir = normalize(GetFrameInfo().GetSunDir().xyz);
		geometric_dark = dot(normal, light_dir) <= 0;
	}

	VSMConstants constants = GetVSMConstants();
	uint4 result = vsm_search_blocker(constants, GetVSMLighting(), wpos, pixel, geometric_dark);
	if (!has_geometry)
		result = uint4(asuint(-1.0), 0, 0, 0); // sentinel: no blocker (matches vsm_search_blocker's own !valid convention)

	// Screen-edge padding: a tile straddling the frame border (frame_size
	// not a multiple of 16) can have threads past dims -- stage 1 already
	// excluded out-of-bounds threads from its own tile-verdict reduction,
	// so this guard just prevents an out-of-range UAV write here too.
	if (in_bounds)
		GetVSMBlockerSearchOutput().GetBlocker_search_result()[pixel] = result;

	// Same sentinel bucket boundaries VSM_ShadowResolve.hlsl's CS_SHADOW_BLUR
	// decodes -- dark sentinels are -2.0 (same level) / -5.0 (via coarser);
	// a real blocker is any non-negative world_delta. Everything else (-1.0
	// no-blocker, -3.0/-4.0 confident_lit) is implicitly lit and
	// contributes to neither flag.
	if (in_bounds)
	{
		float world_delta_or_sentinel = asfloat(result.x);
		bool is_dark         = (world_delta_or_sentinel <= -4.5 && world_delta_or_sentinel > -5.5)
		                     || (world_delta_or_sentinel <= -1.5 && world_delta_or_sentinel > -2.5);
		bool is_real_blocker = world_delta_or_sentinel >= 0;
		if (is_dark)
			InterlockedOr(g_any_dark, 1);
		if (is_real_blocker)
			InterlockedOr(g_any_real_blocker, 1);
	}

	GroupMemoryBarrierWithGroupSync();

	if (groupIndex == 0)
	{
		if (g_any_dark == 0 && g_any_real_blocker == 0)
			GetVSMSearchVerdictAppend().GetConfirmed_lit_tiles().Append(tile_coord);
		else
			GetVSMSearchVerdictAppend().GetBlur_tiles().Append(tile_coord);
	}
}
