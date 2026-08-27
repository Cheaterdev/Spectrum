#include "Common.hlsl"

#include "autogen/VSMLighting.h"
#include "autogen/VSMTileListRead.h"

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
