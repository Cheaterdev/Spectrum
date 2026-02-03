#include "../autogen/FrameGraph_Debug_Texture2D.h"
#include "../autogen/FrameGraph_Debug_Common.h"


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
    
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;
    
    
    int2 utc = (int2(dispatch_thread_id) - GetFrameGraph_Debug_Texture2D().GetOffset()) / GetFrameGraph_Debug_Texture2D().GetScale();
    
 //   [branch]
    if (any(utc > GetFrameGraph_Debug_Texture2D().GetSourceSize()) || any(utc < 0))
    {
        GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = 0.2*(((dispatch_thread_id.x / 8) + (dispatch_thread_id.y / 8)) % 2 == 0);
        return;
    }
       
    
    
    float2 tc = (float2(utc)) / GetFrameGraph_Debug_Texture2D().GetSourceSize();
    
    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = float4(pow(GetFrameGraph_Debug_Texture2D().GetSource()[utc].xyz, 1.0 / 2.2), 1);

}