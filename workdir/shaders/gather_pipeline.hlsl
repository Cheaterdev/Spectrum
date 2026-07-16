
#include "autogen/GatherPipelineGlobal.h"
#include "autogen/SceneData.h"
#include "autogen/FrameInfo.h"


static const GatherPipelineGlobal pip = GetGatherPipelineGlobal();

//static const Buffer<uint> commands = pip.GetCommands();

float dist(float4 plane, float3 pt)
{
    return dot(pt, plane.xyz) + plane.w;
}

float3 local_to_world(float3 local, float4x4 mat)
{
    float4 res = mul(mat, float4(local, 1));
    return res.xyz / res.w;
}


int intersect(Frustum f, AABB aabb, float4x4 mat)
{
    float3 p[8];
    float3 min = aabb.GetMin();
    float3 max = aabb.GetMax();


    p[0] = float3(max.x, max.y, max.z);
    p[1] = float3(max.x, max.y, min.z);
    p[2] = float3(max.x, min.y, max.z);
    p[3] = float3(max.x, min.y, min.z);
    p[4] = float3(min.x, max.y, max.z);
    p[5] = float3(min.x, max.y, min.z);
    p[6] = float3(min.x, min.y, max.z);
    p[7] = float3(min.x, min.y, min.z); 


    [unroll]
    for (int i = 0; i < 8; i++)
        p[i] = local_to_world(p[i], mat);

    bool out_near = false;

    [unroll]
    for (int i = 0; i <6; ++i)
    {

        bool bFullyOut = true;

        for (int j = 0; j < 8; ++j)
        {
            float d = dist(f.GetPlanes(i), p[j]);

            if (d > 0)
                bFullyOut = false;
            else  if (i == 0)
                out_near = true;
        }

        if (bFullyOut)
            return 0;
    }

    return out_near ? 1 : 2;
}
#ifdef  BUILD_FUNC_CS

#include "autogen/GatherPipeline.h"
static const GatherPipeline pipi = GetGatherPipeline();
static const uint ids[8] = (uint[8])pipi.pip_ids;
void get_index(uint id, in CommandData command)
{
#define CHECK(x)    if (ids[x] == id) { pipi.GetCommands(x).Append(command); }
    CHECK(0)
else CHECK(1)
    else CHECK(2)
    else CHECK(3)
    else CHECK(4)
    else CHECK(5)
    else CHECK(6)
    else CHECK(7)
}


[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{
   
        if (dispatchID.x >= pip.GetMeshes_count()[0]) return;
   
        uint id = GetGatherPipelineGlobal().GetCommands().Load(dispatchID.x);

       
        MeshCommandData mesh = GetSceneData().GetMeshes()[id];

#ifdef CHECK_FRUSTUM
        node_data node = GetSceneData().GetNodes()[mesh.GetNode_offset()];
        AABB aabb = node.GetAabb();
        if (!intersect(GetFrameInfo().GetCamera().GetFrustum(), aabb, node.GetNode_global_matrix())) return;
#endif
      
        MaterialCommandData material = GetSceneData().GetMaterials()[mesh.material_id];

 
        CommandData command;
        command.material_cb = material.material_cb;
        command.mesh_cb = mesh.mesh_cb;
         command.meshinstance_cb = mesh.meshinstance_cb;
        command.draw_commands = mesh.draw_commands;
       
        get_index(material.pipeline_id, command);
    //    GetDebugInfo().Log(dispatchID.x, uint4(dispatchID.x, id, 5, 5));

     //   pipi.GetCommands(0).Append(command);
}
#endif

// Consumer dispatch groups for `count` list entries (64 threads per group).
uint dispatch_groups(uint count)
{
    return (count + 63) / 64;
}

#ifdef  BUILD_FUNC_CS_boxes

#include "autogen/GatherBoxes.h"
static const GatherBoxes pipi = GetGatherBoxes();

[numthreads(64, 1, 1)]
void CS_boxes(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    if (dispatchID.x >= pip.GetMeshes_count()[0]) return;

    uint id = GetGatherPipelineGlobal().GetCommands().Load(dispatchID.x);

    MeshCommandData mesh = GetSceneData().GetMeshes()[id];
    node_data node = GetSceneData().GetNodes()[mesh.GetNode_offset()];
    AABB aabb = node.GetAabb();

    int intersection = intersect(GetFrameInfo().GetCamera().GetFrustum(), aabb, node.GetNode_global_matrix());

    if (!intersection) return;
    if (intersection == 1)
    {
        // Near-crossing: box would be clipped — force visible.
        uint idx;
        InterlockedAdd(pipi.GetVisibleCount()[0], 1, idx);
        pipi.GetVisibleMeshes()[idx] = id;

        // Args buffers are clear_uav'ed to zero per stage; the count is bumped
        // monotonically and the constant components restored with benign
        // racing writes (every appender stores the same values).
        InterlockedMax(pipi.GetRenderArgs()[0].counts.x, dispatch_groups(idx + 1));
        pipi.GetRenderArgs()[0].counts.y = 1;
        pipi.GetRenderArgs()[0].counts.z = 1;
        return;
    }

    BoxInfo info;
    info.node_offset = mesh.GetNode_offset();
    info.mesh_id = id;

    uint idx;
    InterlockedAdd(pipi.GetCulledCount()[0], 1, idx);
    pipi.GetCulledMeshes()[idx] = info;

    // Not-covered sentinel for the box raster test — initialized exactly for
    // this stage's candidates, replacing the old full-buffer 999 clear.
    pipi.GetVisible_boxes()[idx] = 999;

    // Bump the consumers' indirect args directly (replaces InitDispatch and
    // the counter->InstanceCount copy). Monotonic, so racing appends are fine.
    InterlockedMax(pipi.GetDrawBoxesArgs()[0].data[1], idx + 1); // InstanceCount
    pipi.GetDrawBoxesArgs()[0].data[0] = 36; // IndexCountPerInstance (unit cube)

    InterlockedMax(pipi.GetGatherMeshesArgs()[0].counts.x, dispatch_groups(idx + 1));
    pipi.GetGatherMeshesArgs()[0].counts.y = 1;
    pipi.GetGatherMeshesArgs()[0].counts.z = 1;
}
#endif



#ifdef  BUILD_FUNC_CS_meshes_from_boxes
#include "autogen/GatherMeshesBoxes.h"
static const GatherMeshesBoxes pipi = GetGatherMeshesBoxes();

[numthreads(64, 1, 1)]
void CS_meshes_from_boxes(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex : SV_GroupIndex
)
{

    if (dispatchID.x >= pip.GetMeshes_count()[0]) return;

    uint id = pipi.GetVisible_boxes().Load(dispatchID.x);

    uint mesh_id = pipi.GetInput_meshes().Load(dispatchID.x).GetMesh_id();

    if (id == 999)
    {
#ifdef INVISIBLE
        uint idx;
        InterlockedAdd(pipi.GetInvisibleCount()[0], 1, idx);
        pipi.GetInvisibleMeshes()[idx] = mesh_id;
        InterlockedMax(pipi.GetRetestArgs()[0].counts.x, dispatch_groups(idx + 1));
        pipi.GetRetestArgs()[0].counts.y = 1;
        pipi.GetRetestArgs()[0].counts.z = 1;
#endif
    }
    else
    {
        uint idx;
        InterlockedAdd(pipi.GetVisibleCount()[0], 1, idx);
        pipi.GetVisibleMeshes()[idx] = mesh_id;
        InterlockedMax(pipi.GetRenderArgs()[0].counts.x, dispatch_groups(idx + 1));
        pipi.GetRenderArgs()[0].counts.y = 1;
        pipi.GetRenderArgs()[0].counts.z = 1;
    }

}
#endif

