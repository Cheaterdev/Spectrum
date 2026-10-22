#include "../autogen/FrameGraph_Debug_Texture2DArray.h"
#include "../autogen/FrameGraph_Debug_Common.h"
#include "channels.hlsli"
#include "stats.hlsli"


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;

    float2 scale = GetFrameGraph_Debug_Texture2DArray().GetScale();
    int2 utc = (int2(dispatch_thread_id) - GetFrameGraph_Debug_Texture2DArray().GetOffset()) / scale;

    float3 color;

    if (any(utc > GetFrameGraph_Debug_Texture2DArray().GetSourceSize()) || any(utc < 0))
    {
        color = 0.2 * (((dispatch_thread_id.x / 8) + (dispatch_thread_id.y / 8)) % 2 == 0);
        pick_store(dispatch_thread_id, 0, int2(0, 0), false);
    }
    else
    {
        uint mip        = GetFrameGraph_Debug_Common().GetSelectedMip();
        uint arrayIndex = GetFrameGraph_Debug_Common().GetSelectedArrayIndex();
        uint2 size = GetFrameGraph_Debug_Texture2DArray().GetSourceSize();
        uint2 mipSize = max(size >> mip, 1);
        float2 tc = float2(utc) / size;
        int2 texel = min(int2(tc * mipSize), int2(mipSize) - 1);

        float4 val = scale.x >= 2.0
            ? GetFrameGraph_Debug_Texture2DArray().GetSource().Load(int4(texel, arrayIndex, mip))
            : GetFrameGraph_Debug_Texture2DArray().GetSource().SampleLevel(linearSampler, float3(tc, arrayIndex), mip);

        pick_store(dispatch_thread_id, val, texel, true);
        color = pow(apply_channels(val, GetFrameGraph_Debug_Common().GetChannelMask()), 1.0 / 2.2);
    }

    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = float4(draw_histogram_overlay(color, dispatch_thread_id), 1);
}
