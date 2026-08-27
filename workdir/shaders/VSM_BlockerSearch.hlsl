#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMTileListRead.h"
#include "autogen/VSMBlockerSearchOutput.h"

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
[numthreads(16, 16, 1)]
void CS_BLOCKER_SEARCH(
	uint3 groupID       : SV_GroupID,
	uint3 groupThreadID : SV_GroupThreadID
)
{
	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

	uint2 tile_origin = GetVSMTileListRead().GetTiles()[groupID.x] * 16;
	uint2 pixel        = tile_origin + groupThreadID.xy;

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
	if (all(pixel < dims))
		GetVSMBlockerSearchOutput().GetBlocker_search_result()[pixel] = result;
}
