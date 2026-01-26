#include "../autogen/FrameGraph_Debug_Texture2D.h"
#include "../autogen/FrameGraph_Debug_Common.h"


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
    
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;
    
    float2 tc = float2(dispatch_thread_id) / GetFrameGraph_Debug_Common().GetTargetSize();
    
    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = float4(GetFrameGraph_Debug_Texture2D().GetSource().SampleLevel(anisoBordeSampler, tc, 0).xyz, 1);

}