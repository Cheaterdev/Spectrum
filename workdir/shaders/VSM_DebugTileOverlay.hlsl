#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMTileListRead.h"
// Reused for its blocker_search_result SRV field only (Instance3 is free
// for this PSO, same as it is for VSM_ShadowResolve's own three -- no
// collision, a given PSO only ever binds the structs its own shader
// references) -- CS_OVERLAY_BLUR never touches shadow_result.
#include "autogen/VSMShadowResolveIO.h"

// Debug view (VSM.ixx's use_vsm_debug_hiz_classify) -- see this PassNode's
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
