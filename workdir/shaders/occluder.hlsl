

#include "autogen/FrameInfo.h"
#include "autogen/SceneData.h"
#include "autogen/DrawBoxes.h"


static const FrameInfo frameInfo = GetFrameInfo();
static const SceneData sceneData = GetSceneData();
static const DrawBoxes gatherBoxes = GetDrawBoxes();

static const RWStructuredBuffer<uint> result = gatherBoxes.GetVisible_meshes();
struct vertex_input
{
    float4 pos : POSITION;
};

struct vertex_output
{
    float4 pos : SV_POSITION;
    unsigned int instance : INSTANCE_ID;
  unsigned int mesh_id : MESH_ID;

};


vertex_output VS( uint index: SV_VertexID, uint instance: SV_InstanceID)
{
    BoxInfo info = gatherBoxes.GetInput_meshes()[instance];
    node_data node = sceneData.GetNodes()[info.GetNode_offset()];
        AABB aabb = node.GetAabb();
        float3 min = aabb.GetMin().xyz;
        float3 max = aabb.GetMax().xyz;

        float3 localpos = gatherBoxes.GetVertices()[index].xyz;
        float4 pos = float4(lerp(min, max, localpos *0.5+0.5),1); 

    matrix node_global_matrix = sceneData.GetNodes()[info.GetNode_offset()].GetNode_global_matrix();
  
    vertex_output o;
    float4 tpos =  mul(node_global_matrix, pos);
    o.pos = mul(frameInfo.GetCamera().GetViewProj(), tpos);
    o.instance = instance;
    o.mesh_id = info.GetMesh_id();
   // result[o.instance] = o.mesh_id;
    return o;
}

//RWStructuredBuffer<uint> view_buffer: register(u0);

[earlydepthstencil]
void PS(vertex_output i)
{
    result[i.instance] = i.mesh_id;
}

/*
float4 PS(vertex_output i) :SV_Target0
{
     //result[i.instance] = i.mesh_id;
   return 1;
}
*/