
#include "autogen/FrameClassification.h"


static const Texture2D<float> frames_tex = GetFrameClassification().GetFrames();


static const AppendStructuredBuffer<uint2> hi = GetFrameClassification().GetHi();
static const AppendStructuredBuffer<uint2> low = GetFrameClassification().GetLow();


static const uint TotalNumThreads =8*8;
groupshared float data[TotalNumThreads];

// One 8x8 group = one 32x32 tile: each thread scans a 4x4 block, then a
// groupshared min-reduction produces the tile minimum and lane 0 appends.
// The dispatch issues exactly tiles_count groups (see VoxelGIGraph.cpp), so
// there are no tail threads to bounds-check at tile level — only the texel
// scan clamps to the texture, so partial edge tiles classify from real data
// instead of phantom out-of-range zeros (OOB reads return 0 -> forced "hi",
// and at small viewports the garbage appends overflowed the hi buffer,
// dropping real tile records -> stale 32x32 blocks / rare TDRs).
[numthreads(8, 8, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{
    uint2 dims;
    frames_tex.GetDimensions(dims.x, dims.y);

    uint2 block_origin = 32 * groupID.xy + 4 * groupThreadID.xy;
    uint2 block_end    = min(block_origin + 4, dims);

    float frames_min = 1; // identity: fully-out-of-range blocks never force "hi"
    for (uint x = block_origin.x; x < block_end.x; x++)
        for (uint y = block_origin.y; y < block_end.y; y++)
            frames_min = min(frames_min, frames_tex[uint2(x, y)]);

    data[groupIndex] = frames_min;
    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint s = TotalNumThreads / 2; s > 0; s >>= 1)
    {
        if (groupIndex < s)
            data[groupIndex] = min(data[groupIndex], data[groupIndex + s]);
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupIndex == 0)
    {
        if (data[0] < 0.9)
            hi.Append(groupID.xy);
        else
            low.Append(groupID.xy);
    }
}
