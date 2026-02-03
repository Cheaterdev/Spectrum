#include "../autogen/FrameGraph_Debug_Texture3D.h"
#include "../autogen/FrameGraph_Debug_Common.h"


#include "../Common.hlsl"


float2 intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax)
{
    float3 tMin = (boxMin - rayOrigin) / rayDir;
    float3 tMax = (boxMax - rayOrigin) / rayDir;
    float3 t1 = min(tMin, tMax);
    float3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return float2(tNear, tFar);
};


[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID)
{
    
    if (any(dispatch_thread_id > GetFrameGraph_Debug_Common().GetTargetSize()))
        return;
    
    

    float2 tc = (float2(dispatch_thread_id)) / GetFrameGraph_Debug_Common().GetTargetSize();
    float3 pos = GetFrameGraph_Debug_Texture3D().GetCamera().GetPosition();
    
    float3 tpos = depth_to_wpos(0.1, tc, GetFrameGraph_Debug_Texture3D().GetCamera().GetInvViewProj());
    float3 dir = normalize(tpos - pos) ;
    
    //float3 res = 0;
    
    
    float2 intersect = intersectAABB(pos, dir, float3(0,0,0)-0.5, float3(1,1,1)-0.5);
    
    
    if (intersect.x > intersect.y)
    {
        GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = 0.2 * (((dispatch_thread_id.x / 8) + (dispatch_thread_id.y / 8)) % 2 == 0);
        return;
    }
    float3 cube_pos_start = pos + intersect.x * dir;
    float3 cube_pos_end = pos + intersect.y * dir;
  
    
    float4 res = 0;
    
    int iters = min(1000, 1000 * (intersect.y - intersect.x));
    for (int i = 0; i < iters; i++)
    {
        float3 cur_pos = lerp(cube_pos_start, cube_pos_end, float(i) / iters);
    
            float4 val = GetFrameGraph_Debug_Texture3D().GetSource().SampleLevel(linearSampler, cur_pos+0.5, 0);
            
        float w = 1 - res.a;
            
         //   val += 1.1;
        res += val * w;
        
    }
        
    
    GetFrameGraph_Debug_Common().GetTarget()[dispatch_thread_id] = float4(pow(res.xyz, 1.0f / 2.2f), 1);

}