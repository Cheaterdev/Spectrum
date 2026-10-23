#include "../autogen/FrameGraph_Debug_Texture2DArray.h"
#include "../autogen/FrameGraph_Debug_Common.h"
#include "stats.hlsli"

[numthreads(8, 8, 1)]
void CS(uint2 g : SV_DispatchThreadID)
{
    if (GetFrameGraph_Debug_Common().GetFlags() & FLAG_CLEAR)
    {
        stats_clear(g);
        return;
    }

    uint mip = GetFrameGraph_Debug_Common().GetSelectedMip();
    uint2 mipSize = max(GetFrameGraph_Debug_Texture2DArray().GetSourceSize() >> mip, 1);
    uint2 grid = min(mipSize, HIST_MAX_GRID);

    if (any(g >= grid))
        return;

    if (all(g == 0))
        GetFrameGraph_Debug_Common().GetStats()[STATS_TOTAL] = grid.x * grid.y;

    int2 texel = int2(g * mipSize / grid);
    stats_accumulate(GetFrameGraph_Debug_Texture2DArray().GetSource().Load(int4(texel, GetFrameGraph_Debug_Common().GetSelectedArrayIndex(), mip)));
}
