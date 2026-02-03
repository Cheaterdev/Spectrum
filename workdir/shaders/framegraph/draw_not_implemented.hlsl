
#include "../autogen/FrameGraph_Debug_Common.h"


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
 
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;
     
    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = 0.8 * (((dispatch_thread_id.x / 8) + (dispatch_thread_id.y / 8)) % 2 == 0);
}