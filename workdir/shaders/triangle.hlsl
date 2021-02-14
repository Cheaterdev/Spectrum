
struct vertex_output
{
    float4 pos : SV_POSITION;
    float3 wpos : POSITION;
    float3 normal : NORMAL;
    float3 binormal : BINORMAL;
    float3 tangent : TANGENT;
    float2 tc : TEXCOORD;
    float4 cur_pos : CUR_POSITION;

    float4 prev_pos : PREV_POSITION;
    float dist : DISTANCE;

};

#include "Common.hlsl"

#include "autogen/FrameInfo.h"
#include "autogen/MeshInfo.h" 
#include "autogen/SceneData.h"


static const FrameInfo frameInfo = GetFrameInfo();
static const MeshInfo meshInfo = GetMeshInfo();
static const SceneData sceneData = GetSceneData();


vertex_output transform(matrix node_global_matrix, matrix node_global_matrix_prev, Camera camera, mesh_vertex_input i)
{
    vertex_output o;
    float4 tpos = mul(node_global_matrix, float4(i.pos, 1));
    o.wpos.xyz = tpos.xyz / tpos.w;
    o.pos = mul(frameInfo.GetCamera().GetViewProj(), tpos);
    o.normal = normalize(mul((float3x3)node_global_matrix, i.normal));
    o.tangent = normalize(mul((float3x3) node_global_matrix, i.tangent.xyz) * i.tangent.w);
    o.binormal = normalize(cross(i.normal, i.tangent.xyz) * i.tangent.w);
    o.tc = i.tc;
    float3 v = o.wpos.xyz - frameInfo.GetCamera().GetPosition();
    float dist = length(v);
    o.dist = (1 - clamp(dist, 10, 40) / 40);
    o.dist *= 1 + pow(1 - abs(dot(o.normal, v / dist)), 4);
    o.dist = clamp(o.dist, 0, 1);
    o.cur_pos = o.pos;

    float4 ppos = mul(node_global_matrix_prev, float4(i.pos, 1));

    o.prev_pos = mul(frameInfo.GetPrevCamera().GetViewProj(), ppos);
    return o;
}



vertex_output VS(uint index: SV_VertexID)
{
    	//matrix m = { {1,0,0,0}, { 0,1,0,0 }, { 0,0,1,0 }, {0,0,0,1 } };
    node_data node= sceneData.GetNodes()[meshInfo.GetNode_offset()];
    matrix m = node.GetNode_global_matrix();
    return transform(node.GetNode_global_matrix(), node.GetNode_global_matrix_prev(), frameInfo.GetCamera(), sceneData.GetVertexes()[meshInfo.GetVertex_offset()+ index]);
}

