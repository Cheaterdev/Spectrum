#include "autogen/VoxelScreen.h"
#include "autogen/FrameInfo.h"
#include "SS_Shadow.hlsl"
#include "Common.hlsl"
#include "autogen/Raytracing.h"
#include "autogen/workgraph/WorkGR.h"

NODE_ClassifyPixels_Node(
    WG_DISPATCH_INPUT(GraphInput) inputData,
    uint3 LocalThreadId : SV_GroupThreadID,
    uint3 WorkGroupId : SV_GroupID,
    uint3 Dtid : SV_DispatchThreadID
    WG_NODE_OUTPUT_ClassifyPixels_Node
)
{
    DispatchParameters params = CreateDispatchParameters();
    params.WaveOffset = inputData.Get().WaveOffset;

    Result v = WriteScreenSpaceShadow(params, WG_GetGroupID(WorkGroupId), LocalThreadId);

    WG_OUTPUT_RECORDS(TileRecord, tileRecord, Shadows_Node, v.good && !v.shadow ? 1 : 0);

    if (v.good)
    {
        if (v.shadow)
            params.GetOutputTexture()[uint2(v.pixel_pos)] = 0;
        else
        {

            params.GetOutputTexture()[uint2(v.pixel_pos)] = 0.3;
             WG_SET_RECORD(tileRecord, tileXY, uint2(v.pixel_pos));
        }
           
    }

    WG_OUTPUT_COMPLETE(tileRecord);
}

NODE_Shadows_Node(
    WG_THREAD_INPUT(TileRecord) inputData
)
{
    uint2 pixel_pos = inputData.Get().tileXY;
    const Camera camera = CreateFrameInfo().GetCamera();
    GBuffer gbuffer = CreateVoxelScreen().GetGbuffer();
    const float3 sunDir = CreateFrameInfo().GetSunDir();
    const Raytracing raytracing = CreateRaytracing();

    float raw_z = gbuffer.GetDepth()[pixel_pos.xy];
    float2 dims;
    gbuffer.GetAlbedo().GetDimensions(dims.x, dims.y);

    float3 pos = depth_to_wpos(raw_z, float2(pixel_pos) / dims, camera.GetInvViewProj());
    	float3 normal = normalize(gbuffer.GetNormals()[pixel_pos].xyz * 2 - 1);


    DispatchParameters params = CreateDispatchParameters();

    RayDesc ray;
    ray.Origin = pos + normal*0.001;
    ray.Direction = sunDir;
    ray.TMin = 0.2;
    ray.TMax = 100000;

    RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    rayQuery.TraceRayInline(raytracing.GetScene(), RAY_FLAG_NONE, 0xFF, ray);
    rayQuery.Proceed();

    float shadow = (rayQuery.CommittedStatus() == COMMITTED_NOTHING) ? 1.0 : 0.0;
    params.GetOutputTexture()[pixel_pos] = shadow;
}
