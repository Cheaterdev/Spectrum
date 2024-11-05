#include "autogen/WorkGraphTest.h"
#include "autogen/tables/GraphInput.h"
#include "autogen/VoxelScreen.h"
#include "autogen/FrameInfo.h"
#include "SS_Shadow.hlsl"
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
    params.WaveOffset = inputData.Get().WaveOffset;
    
     Result v=  WriteScreenSpaceShadow(params, WorkGroupId, LocalThreadId);


 ThreadNodeOutputRecords <TileRecord>tileRecord = Shadows_Node.GetThreadNodeOutputRecords(v.good&&!v.shadow ? 1 : 0);
    if (v.good)
    {

        if(v.shadow)
        {
	      	params.GetOutputTexture()[uint2(v.pixel_pos)] = 0;
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
    
    DispatchParameters params = CreateDispatchParameters();
    uint2 pixel_pos = inputData.Get().tileXY;
    params.GetOutputTexture()[pixel_pos] = 1;
    //// use the record data to reconstruct screen position for this thread
    //const uint2 screenPos = inputData.Get().tileXY * uint2(8, 8) + groupThreadId;
     
    //const WorkGraphTest data = CreateWorkGraphTest();
    //const GBuffer gbuffer = CreateVoxelScreen().GetGbuffer();
    //const Camera camera = CreateFrameInfo().GetCamera();
    //const float3 sunDir = CreateFrameInfo().GetSunDir();
    //float3 normal = normalize(gbuffer.GetNormals()[screenPos].xyz * 2 - 1);
    //float NdotL = dot(normal, sunDir.xyz);
 
    
    //{    
    //    //this is a valid hit, write a shadow factor of zero to the shadowmask
    //    data.GetOutput()[screenPos.xy] = NdotL;
    //}
 
    //mark record as done
 //   threadRecord.OutputComplete();
}