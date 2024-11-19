#include "autogen/WorkGraphTest.h"
#include "autogen/tables/GraphInput.h"
#include "autogen/VoxelScreen.h"
#include "autogen/FrameInfo.h"
#include "SS_Shadow.hlsl"
#include "Common.hlsl"
#include "autogen/Raytracing.h"

    struct TileRecord
    {
        uint2 tileXY;
    };
 
    groupshared unsigned int g_allbackfacing;




    [Shader("node")]
    [NodeLaunch("broadcasting")]
    [NodeIsProgramEntry]
    [NodeMaxDispatchGrid(256, 64, 64)]
    [numthreads(WAVE_SIZE, 1, 1)]
void ClassifyPixels_Node(
DispatchNodeInputRecord< GraphInput> inputData,

  uint3 LocalThreadId : SV_GroupThreadID,
    uint3 WorkGroupId : SV_GroupID,
    uint3 Dtid : SV_DispatchThreadID,
    
    [MaxRecords(WAVE_SIZE)] NodeOutput<TileRecord> Shadows_Node
)
{

    DispatchParameters params = CreateDispatchParameters();
    params.WaveOffset = inputData.Get().
    WaveOffset;
    
     Result v=  WriteScreenSpaceShadow(params, WorkGroupId, LocalThreadId);


 ThreadNodeOutputRecords <TileRecord>tileRecord = Shadows_Node.GetThreadNodeOutputRecords(v.good&&!v.shadow ? 1 : 0);
uint2 tc = uint2(v.pixel_pos);

    if (v.good)
    {

        if(v.shadow)
        {
	      	params.GetOutputTexture()[tc] = 0;
        }
        else
        {
            	//params.GetOutputTexture()[uint2(v.pixel_pos)] = 1;
            tileRecord[0].tileXY = uint2(v.pixel_pos);

        }
	
    }

        tileRecord.OutputComplete();
//const WorkGraphTest data = CreateWorkGraphTest();
//const GBuffer gbuffer = CreateVoxelScreen().GetGbuffer();
//const Camera camera = CreateFrameInfo().GetCamera();
//const float3 sunDir = CreateFrameInfo().GetSunDir();

//    if ( groupThreadIndex == 0 )
//    {
//        g_allbackfacing = 1; // initialise group shared memory
//    }
 
//    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE|GROUP_SYNC);
 
//uint2 screenPos = globalThreadID.xy;
 
//float3 normal = normalize(gbuffer.GetNormals()[screenPos].xyz * 2 - 1);
 
//float NdotL = dot(normal, sunDir.xyz);
 
//bool backfacing = NdotL <= 0;
 
//    // check if all threads in the wave are backfacing
//bool allBackfacing = WaveActiveAllTrue(backfacing);
 
//    //do an interlocked operation only for the first thread in the wave
//    if ( WaveIsFirstLane() )
//    {
//int previous;
//        InterlockedAnd(g_allbackfacing, allBackfacing ? 1 : 0, previous);
//    }
 
//    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE|GROUP_SYNC);
 
    // create a record for this tile
  //  GroupNodeOutputRecords<TileRecord> tileRecord = Shadows_Node.GetGroupNodeOutputRecords(0);
 
    //if ( !g_allbackfacing )
    //{
    //    if (groupThreadIndex == 0 )
    //          tileRecord[0].tileXY = groupId; // if not all backfacing write tile coordinate
    //}
    //else
    {   
      // data.GetOutput()[screenPos] = 0; 
    }
      
    // mark the node record as complete.

}


struct PixelRecord
{

    uint2 screenPos;
    float3 rayDir;
    float3 rayOrigin;
};
 
[Shader("node")]
[NodeLaunch("thread")]
void Shadows_Node(
    ThreadNodeInputRecord< TileRecord> inputData
    //[MaxRecords(64)] NodeOutput<PixelRecord> ShadowsDXR_Node
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



	DispatchParameters params = CreateDispatchParameters();




	RayDesc ray;
	ray.Origin = pos;

	ray.Direction = sunDir;
	ray.TMin = 0.1;
	ray.TMax = 100000;

	RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;

	rayQuery.TraceRayInline(raytracing.GetScene(), RAY_FLAG_NONE, 0xFF, ray);
	rayQuery.Proceed();

	float shadow = (rayQuery.CommittedStatus() == COMMITTED_NOTHING) ? 1.0 : 0.0;

	params.GetOutputTexture()[pixel_pos] = shadow;

}
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeIsProgramEntry]
[NodeMaxDispatchGrid(256, 64, 64)]
[numthreads(WAVE_SIZE, 1, 1)]
void ClassifyPixels_Node2(
DispatchNodeInputRecord< GraphInput> inputData,

  uint3 LocalThreadId : SV_GroupThreadID,
    uint3 WorkGroupId : SV_GroupID,
    uint3 Dtid : SV_DispatchThreadID,
    
    [MaxRecords(WAVE_SIZE)] NodeOutput<TileRecord> Shadows_Node
)
{
    

 ThreadNodeOutputRecords <TileRecord>tileRecord = Shadows_Node.GetThreadNodeOutputRecords(0);

    
        tileRecord.OutputComplete();
}