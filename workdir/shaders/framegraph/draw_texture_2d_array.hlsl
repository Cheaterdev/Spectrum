#include "../autogen/FrameGraph_Debug_Texture2DArray.h"
#include "../autogen/FrameGraph_Debug_Common.h"


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;

    int2 utc = (int2(dispatch_thread_id) - GetFrameGraph_Debug_Texture2DArray().GetOffset()) / GetFrameGraph_Debug_Texture2DArray().GetScale();

    if (any(utc > GetFrameGraph_Debug_Texture2DArray().GetSourceSize()) || any(utc < 0))
    {
        GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = 0.2*(((dispatch_thread_id.x / 8) + (dispatch_thread_id.y / 8)) % 2 == 0);
        return;
    }

    uint mip        = GetFrameGraph_Debug_Common().GetSelectedMip();
    uint arrayIndex = GetFrameGraph_Debug_Common().GetSelectedArrayIndex();
    float2 tc = float2(utc) / GetFrameGraph_Debug_Texture2DArray().GetSourceSize();
    float4 val = GetFrameGraph_Debug_Texture2DArray().GetSource().SampleLevel(linearSampler, float3(tc, arrayIndex), mip);

    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = float4(pow(max(val.xyz, 0.0), 1.0 / 2.2), 1);
}
