#include "autogen/GatherPipeline.h"
#include "autogen/GatherPipelineGlobal.h"
#include "autogen/SceneData.h"
#include "autogen/FrameInfo.h"

static const GatherPipelineGlobal pip = GetGatherPipelineGlobal();
static const GatherPipeline pipi = GetGatherPipeline();
static const Buffer<uint> commands = GetSceneData().GetCommands();

//static const AppendStructuredBuffer<CommandData> commands[8] = pip.GetCommands();

static const uint ids[8] = (uint[8])pipi.cb.pip_ids;
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

groupshared CommandData data[31228][8];

float dist(float4 plane, float3 pt)
{
    return dot(pt, plane.xyz) + plane.w;
}

float3 local_to_world(float3 local, float4x4 mat)
{
    float4 res = mul(mat, float4(local, 1));
    return res.xyz / res.w;
}
bool intersect(Frustum f, AABB aabb, float4x4 mat)
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


  //  [unroll]
    for (int i = 0; i < 8; i++)
        p[i] = local_to_world(p[i], mat);

    bool out_near = false;

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
            return false;
    }

    return true;
}

[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{

    if (dispatchID.x >= pip.GetMeshes_count()) return;

        uint id = commands.Load(dispatchID.x);
        MeshCommandData mesh =  pip.GetMeshes()[id];

        node_data node = GetSceneData().GetNodes()[mesh.GetNode_offset()];
        AABB aabb = node.GetAabb();

        if (!intersect(GetFrameInfo().GetCamera().GetFrustum(), aabb, node.GetNode_global_matrix())) return;


        MaterialCommandData material = pip.GetMaterials()[mesh.cb.material_id];
        CommandData command;
        command.cb.material_cb = material.cb.material_cb;
        command.cb.mesh_cb = mesh.cb.mesh_cb;
        command.cb.draw_commands = mesh.cb.draw_commands;
       
        get_index(material.cb.pipeline_id, command);

}
