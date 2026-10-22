#include "../autogen/FrameGraph_Debug_Texture2D.h"
#include "../autogen/FrameGraph_Debug_Common.h"
#include "stats.hlsli"

// One thread per sample of a grid of at most HIST_MAX_GRID^2 points spread over the selected mip, so the cost
// stays flat for big textures. resource_preview clears the stats buffer before this runs.
[numthreads(8, 8, 1)]
void CS(uint2 g : SV_DispatchThreadID)
{
    if (GetFrameGraph_Debug_Common().GetFlags() & FLAG_CLEAR)
    {
        uint word = g.y * 128 + g.x;
        if (word < STATS_WORDS)
            GetFrameGraph_Debug_Common().GetStats()[word] = 0;
        return;
    }

    uint mip = GetFrameGraph_Debug_Common().GetSelectedMip();
    uint2 mipSize = max(GetFrameGraph_Debug_Texture2D().GetSourceSize() >> mip, 1);
    uint2 grid = min(mipSize, HIST_MAX_GRID);

    if (any(g >= grid))
        return;

    if (all(g == 0))
        GetFrameGraph_Debug_Common().GetStats()[STATS_TOTAL] = grid.x * grid.y;

    int2 texel = int2(g * mipSize / grid);
    float4 v = GetFrameGraph_Debug_Texture2D().GetSource().Load(int3(texel, mip));

    uint ignored;
    [unroll]
    for (uint c = 0; c < 4; ++c)
        InterlockedAdd(GetFrameGraph_Debug_Common().GetStats()[STATS_HIST + c * 256 + hist_bin(v[c])], 1, ignored);
}
