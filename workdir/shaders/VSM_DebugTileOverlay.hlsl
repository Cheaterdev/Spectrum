#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMConstants.h"
#include "autogen/FrameInfo.h"
#include "autogen/VSMTileListRead.h"
// Reused for its blocker_search_result SRV field only (Instance3 is free
// for this PSO, same as it is for VSM_ShadowResolve's own three -- no
// collision, a given PSO only ever binds the structs its own shader
// references) -- CS_OVERLAY_BLUR never touches shadow_result.
#include "autogen/VSMShadowResolveIO.h"

static const GBuffer gbuffer = GetVSMLighting().GetGbuffer();

// Only actually needed by CS_OVERLAY_PAGE_GRID (get_vsm_debug_page_grid_color)
// -- pulls in VSM_impl.hlsl transitively (get_vsm_level/get_vsm_slot), same
// as every other file in this pipeline that needs the debug helpers.
#include "VSM_impl_resolve.hlsl"

// Debug view (VSM.ixx's vsm_debug_view == HizClassify) -- see this PassNode's
// own comment in vsm.sig for why this reads the REAL tile lists directly
// instead of guessing from the final shadow value. Only ever dispatched
// when the toggle is on; paints a flat color onto VSMLighting's `result`
// field (the same RWTexture2D<float4> ResultTexture VSM_Combine already
// wrote real shading into) for every pixel in the appended tile.
void overlay_tile(uint3 groupID, uint3 groupThreadID, float4 color)
{
	uint2 dims;
	GetVSMLighting().GetResult().GetDimensions(dims.x, dims.y);

	uint2 tile_origin = GetVSMTileListRead().GetTiles()[groupID.x] * 16;
	uint2 pixel        = tile_origin + groupThreadID.xy;

	if (all(pixel < dims))
		GetVSMLighting().GetResult()[pixel] = color;
}

[numthreads(16, 16, 1)]
void CS_OVERLAY_LIT(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	overlay_tile(groupID, groupThreadID, float4(0, 1, 0, 1));
}

[numthreads(16, 16, 1)]
void CS_OVERLAY_DARK(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	overlay_tile(groupID, groupThreadID, float4(0, 0, 1, 1));
}

// Stage 2's own post-search verdict (see VSMSearchVerdictAppend's own
// comment in vsm.sig) -- confirmed_lit_tiles: every pixel individually
// resolved lit after the REAL search ran, distinct color (cyan) from
// lit_tiles' green so it's visible how much of the frame stage 1's cheap
// classify alone couldn't prove, but the real search still confirmed lit.
[numthreads(16, 16, 1)]
void CS_OVERLAY_CONFIRMED_LIT(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	overlay_tile(groupID, groupThreadID, float4(0, 1, 1, 1));
}

// blur_tiles: PER-PIXEL treatment, not a flat color -- a tile lands here
// because SOME pixel in it was genuinely ambiguous, but most of the OTHERS
// still resolved via a sentinel (see VSMSearchVerdictAppend's own comment
// in vsm.sig) even though the whole tile still had to dispatch. Reads the
// same packed VSM_BlockerSearchResult data CS_SHADOW_BLUR itself decodes
// (VSM_ShadowResolve.hlsl) and mirrors its exact sentinel buckets -- dark
// green/blue (half brightness of lit_tiles/dark_tiles' own colors) for a
// sentinel-resolved pixel, distinguishing "optimization fired at the
// per-pixel level within a dispatched tile" from both the bright
// tile-level colors (stage 1/stage 2's own list-level skips) and from a
// real blocker (>=0), which is left completely untouched so the real
// blurred shadow shows through unmodified -- that's the one case with no
// optimization to show.
[numthreads(16, 16, 1)]
void CS_OVERLAY_BLUR(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	uint2 dims;
	GetVSMLighting().GetResult().GetDimensions(dims.x, dims.y);

	uint2 tile_origin = GetVSMTileListRead().GetTiles()[groupID.x] * 16;
	uint2 pixel        = tile_origin + groupThreadID.xy;
	if (any(pixel >= dims))
		return;

	uint4 blocker_packed = GetVSMShadowResolveIO().GetBlocker_search_result()[pixel];
	float world_delta_or_sentinel = asfloat(blocker_packed.x);

	// Same bucket boundaries as VSM_ShadowResolve.hlsl's CS_SHADOW_BLUR --
	// see that file's own comment for what each sentinel means.
	bool is_dark = (world_delta_or_sentinel <= -4.5 && world_delta_or_sentinel > -5.5)
	            || (world_delta_or_sentinel <= -1.5 && world_delta_or_sentinel > -2.5);
	bool is_lit  = (world_delta_or_sentinel <= -3.5 && world_delta_or_sentinel > -4.5)
	            || (world_delta_or_sentinel <= -2.5 && world_delta_or_sentinel > -3.5)
	            || (world_delta_or_sentinel < 0 && world_delta_or_sentinel > -1.5);

	if (is_dark)
		GetVSMLighting().GetResult()[pixel] = float4(0, 0, 0.35, 1);
	else if (is_lit)
		GetVSMLighting().GetResult()[pixel] = float4(0, 0.35, 0, 1);
	// else: real blocker (world_delta_or_sentinel >= 0) -- leave untouched,
	// the real blurred shadow already written by CS_SHADOW_BLUR shows
	// through as-is.
}

// Moved here from VSM.hlsl's combine_result now that VSM_Combine no longer
// runs at all when use_vsm_penumbra is on (see this PassNode's own comment
// in vsm.sig) -- both full-screen, not tile-list-driven, since these debug
// views don't care which classify bucket a pixel landed in. Mutually
// exclusive with each other and with the tile-classify overlay above (see
// VSM.cpp's m_debugoverlay_render for the precedence).

// Debug view (VSM.ixx's vsm_debug_view == PageGrid): flat per-level color,
// checkerboard-darkened by page position within that level -- see
// get_vsm_debug_page_grid_color's own comment (VSM_impl_resolve.hlsl).
[numthreads(16, 16, 1)]
void CS_OVERLAY_PAGE_GRID(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetVSMLighting().GetResult().GetDimensions(dims.x, dims.y);
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	float raw_z = gbuffer.GetDepth().SampleLevel(pointClampSampler, tc, 0);
	if (raw_z == 0)
		return;

	Camera camera = GetFrameInfo().GetCamera();
	float3 wpos = depth_to_wpos(raw_z, tc, camera.GetInvViewProj());
	VSMConstants c = GetVSMConstants();
	GetVSMLighting().GetResult()[DTid.xy] = float4(get_vsm_debug_page_grid_color(c, wpos), 1);
}

// Debug view (VSM.ixx's vsm_debug_view == RtxReference): bypass VSM's own
// shadow entirely and show RTXShadow's own denoised full-RT shadow mask as
// grayscale.
[numthreads(16, 16, 1)]
void CS_OVERLAY_RTX_REFERENCE(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	GetVSMLighting().GetResult().GetDimensions(dims.x, dims.y);
	if (any(DTid.xy >= dims))
		return;

	float2 tc = (float2(DTid.xy) + 0.5) / float2(dims);
	float rtx_shadow = GetVSMLighting().GetRtx_shadow_mask().SampleLevel(pointClampSampler, tc, 0);
	GetVSMLighting().GetResult()[DTid.xy] = float4(rtx_shadow, rtx_shadow, rtx_shadow, 1);
}
