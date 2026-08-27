#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMBlockerTilesAppend.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

// vsm_classify_blocker (VSM_impl_search.hlsl) is the cheap half of
// vsm_search_blocker, factored out specifically so this pass can reuse it
// without paying for the tap loop / quad-merge machinery it never needs.
// Pulls in autogen/VSMPageHiZ.h itself -- don't also include it directly
// here, this HLSL toolchain has no include guards and a double-include
// redefines the same constant-buffer slot (confirmed the hard way).
#include "VSM_impl_search.hlsl"

// Stage 1 (Phase 5.18 Part A follow-up, take 4): groupshared tile
// classification, mirroring VoxelGIGraph's own FrameClassification
// hi/low-tile-list pattern -- one 16x16 group per screen tile, each thread
// classifies its own pixel via vsm_classify_blocker, and a groupshared
// reduction decides the WHOLE tile's verdict. Writes NOTHING pixel-shaped
// -- purely classification and bucketing; see vsm.sig's own PassNode
// comment for why (a resource written by several independent downstream
// passes with no data dependency between them isn't reliably visible to
// FrameGraph's dependency resolution -- confirmed live, twice, earlier this
// session). Thread 0 appends the tile's coordinate to exactly one of three
// lists:
//   - lit_tiles:    every in-bounds pixel confidently lit (includes
//                   sky/no-geometry/unresolvable, matching the existing
//                   -1.0 fallback convention)
//   - dark_tiles:   every in-bounds pixel confidently dark (includes
//                   geometric self-shadow, NdotL <= 0)
//   - search_tiles: anything not cleanly uniform -- at least one genuinely
//                   ambiguous pixel, OR a tile with BOTH confidently-lit
//                   AND confidently-dark pixels but ZERO ambiguous ones (a
//                   hard boundary with no true penumbra pixel in this
//                   tile) -- stage 3's full-lit/full-shadow PSOs write a
//                   flat value with no per-pixel check, so a mixed tile
//                   can't go in either uniform bucket.
groupshared uint g_any_lit;
groupshared uint g_any_dark;
groupshared uint g_any_check;

[numthreads(16, 16, 1)]
void CS_BLOCKER_CLASSIFY(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID    : SV_DispatchThreadID,
	uint  groupIndex    : SV_GroupIndex
)
{
	if (groupIndex == 0)
	{
		g_any_lit = 0;
		g_any_dark = 0;
		g_any_check = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	uint2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

	uint2 pixel     = dispatchID.xy;
	bool  in_bounds = all(pixel < dims);

	float2 tc     = (float2(pixel) + 0.5) / float2(dims);
	float  raw_z  = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	bool   has_geometry = in_bounds && (raw_z != 0);

	Camera camera = GetFrameInfo().GetCamera();
	float3 wpos = has_geometry ? depth_to_wpos(raw_z, tc, camera.GetInvViewProj()) : camera.GetPosition();

	// Same geometric self-shadow early-out as VSM_BlockerSearch.hlsl's own
	// CS_BLOCKER_SEARCH -- see its comment for why.
	bool geometric_dark = false;
	if (has_geometry)
	{
		float3 normal = normalize(gbuffer.GetNormals().SampleLevel(pointClampSampler, tc, 0).xyz * 2 - 1);
		float3 light_dir = normalize(GetFrameInfo().GetSunDir().xyz);
		geometric_dark = dot(normal, light_dir) <= 0;
	}

	VSMConstants constants = GetVSMConstants();

	// 0 = lit, 1 = dark, 2 = check -- only meaningful when in_bounds.
	uint verdict = 0;
	if (has_geometry)
	{
		VSMBlockerClassifyResult cls = vsm_classify_blocker(constants, GetVSMLighting(), wpos, geometric_dark);
		if (!cls.valid)
			verdict = 0; // matches vsm_search_blocker's own !valid -> -1.0 fallback
		else if (cls.confident_lit)
			verdict = 0;
		else if (cls.confident_dark)
			verdict = 1;
		else
			verdict = 2;
	}
	// else: sky/no-geometry -- verdict stays 0 (lit), matches the -1.0
	// sentinel sky pixels have always gotten.

	if (in_bounds)
	{
		if (verdict == 2)
			InterlockedOr(g_any_check, 1);
		else if (verdict == 1)
			InterlockedOr(g_any_dark, 1);
		else
			InterlockedOr(g_any_lit, 1);
	}

	GroupMemoryBarrierWithGroupSync();

	if (groupIndex == 0)
	{
		if (g_any_check != 0 || (g_any_lit != 0 && g_any_dark != 0))
			GetVSMBlockerTilesAppend().GetSearch_tiles().Append(groupID.xy);
		else if (g_any_dark != 0)
			GetVSMBlockerTilesAppend().GetDark_tiles().Append(groupID.xy);
		else
			GetVSMBlockerTilesAppend().GetLit_tiles().Append(groupID.xy);
	}
}
