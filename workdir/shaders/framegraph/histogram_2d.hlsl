#include "../autogen/FrameGraph_Debug_Texture2D.h"
#include "../autogen/FrameGraph_Debug_Common.h"
#include "stats.hlsli"

// One thread per sample of a grid of at most HIST_MAX_GRID^2 points spread over the selected mip, so the cost
// stays flat for big textures. resource_preview runs it twice: first with FLAG_CLEAR (zero the stats words), then to accumulate.
[numthreads(8, 8, 1)]
void CS(uint2 g : SV_DispatchThreadID)
{
    if (GetFrameGraph_Debug_Common().GetFlags() & FLAG_CLEAR)
    {
        stats_clear(g);
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
    stats_accumulate(GetFrameGraph_Debug_Texture2D().GetSource().Load(int3(texel, mip)));
}
