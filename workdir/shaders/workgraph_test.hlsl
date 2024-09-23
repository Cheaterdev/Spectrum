#include "autogen/WorkGraphTest.h"
#include "autogen/tables/GraphInput.h"


struct TileRecord 
{
    uint2 tileXY;
};
 
groupshared unsigned int g_allbackfacing;
 
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeIsProgramEntry]
[NodeDispatchGrid(16, 16, 1)] // This will be overridden during pipeline creation
[numthreads(8, 8, 1)]
void ClassifyPixels_Node(
//DispatchNodeInputRecord< GraphInput> inputData,

    in uint3 globalThreadID : SV_DispatchThreadID,
    in uint2 groupId : SV_GroupID,
    in uint groupThreadIndex : SV_GroupIndex,
    [MaxRecords(1)] NodeOutput<TileRecord> Shadows_Node
)
{
static const WorkGraphTest data = CreateWorkGraphTest();

    if ( groupThreadIndex == 0 )
    {
        g_allbackfacing = 1; // initialise group shared memory
    }
 
    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE|GROUP_SYNC);
 
    uint2 screenPos = globalThreadID.xy;
 
float3 normal = 1;
 
float NdotL = 1;//dot(normal, lightDir.xyz);
 
    bool backfacing = NdotL <= 0;
 
    // check if all threads in the wave are backfacing
    bool allBackfacing = WaveActiveAllTrue(backfacing);
 
    //do an interlocked operation only for the first thread in the wave
    if ( WaveIsFirstLane() )
    {
        int previous;
        InterlockedAnd(g_allbackfacing, allBackfacing ? 1 : 0, previous);
    }
 
    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE|GROUP_SYNC);
 
    // create a record for this tile
    GroupNodeOutputRecords<TileRecord> tileRecord = Shadows_Node.GetGroupNodeOutputRecords(g_allbackfacing ? 0 : 1);
 
    if ( !g_allbackfacing )
    {
        if (groupThreadIndex == 0 )
              tileRecord[0].tileXY = groupId; // if not all backfacing write tile coordinate
    }
    else
    {   
       
    }
      data.GetOutput()[screenPos] = 1; // else add a zero shadowfactor 
    // mark the node record as complete.
    tileRecord.OutputComplete();
}


struct PixelRecord
{

uint2 screenPos;
float3 rayDir;
float3 rayOrigin;
};
 
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(8, 8, 1)]
void Shadows_Node(
    DispatchNodeInputRecord< TileRecord> inputData,
    uint2 groupThreadId : SV_GroupThreadID,
    uint threadIndex : SV_GroupIndex,
    uint2 groupId : SV_GroupID
    //[MaxRecords(64)] NodeOutput<PixelRecord> ShadowsDXR_Node
)
{
    // use the record data to reconstruct screen position for this thread
const uint2 screenPos = inputData.Get().tileXY * uint2(8, 8) + groupThreadId;
     
static const WorkGraphTest data = CreateWorkGraphTest();

    {    
        //this is a valid hit, write a shadow factor of zero to the shadowmask
         data.GetOutput()[screenPos.xy] = 1;
    }
 
    //mark record as done
 //   threadRecord.OutputComplete();
}