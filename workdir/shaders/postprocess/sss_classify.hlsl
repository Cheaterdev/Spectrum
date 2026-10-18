#include "../common/common.hlsl"
#include "../common/shading_model.hlsl"

#include "../autogen/SSSClassifyParams.h"
#include "../autogen/SSSTilesAppend.h"

// One 16x16 group per screen tile: if any pixel of the tile is SUBSURFACE the
// tile's position goes into the list the blur passes dispatch over (see
// sss.prism). A tile only needs to be listed when it contains such a pixel:
// the blur reads neighbouring tiles but only ever writes its own pixels.

static const GBuffer gbuffer = GetSSSClassifyParams().GetGbuffer();

groupshared uint tile_has_subsurface;

[numthreads(16, 16, 1)]
void CS_CLASSIFY(uint3 groupID : SV_GroupID, uint3 dispatchID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
		tile_has_subsurface = 0;

	GroupMemoryBarrierWithGroupSync();

	float2 dims;
	gbuffer.GetDepth().GetDimensions(dims.x, dims.y);

	if (all(dispatchID.xy < uint2(dims)))
		if (gbuffer.GetDepth()[dispatchID.xy] != 0 && material_model(gbuffer.GetMaterial()[dispatchID.xy]) == SHADING_MODEL_SUBSURFACE)
			InterlockedOr(tile_has_subsurface, 1);

	GroupMemoryBarrierWithGroupSync();

	if (groupIndex == 0 && tile_has_subsurface != 0)
		GetSSSTilesAppend().GetTiles().Append(groupID.xy);
}
