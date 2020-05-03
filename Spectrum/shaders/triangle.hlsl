
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
#include "autogen/MeshData.h"
#include "autogen/SceneData.h"


static const FrameInfo frameInfo = GetFrameInfo();
static const MeshInfo meshInfo = GetMeshInfo();
static const MeshData meshData = GetMeshData();
static const SceneData sceneData = GetSceneData();


vertex_output transform(matrix node_global_matrix, camera_info camera, mesh_vertex_input_cb i)
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

    o.prev_pos = mul(frameInfo.GetPrevCamera().GetViewProj(), tpos);
    return o;
}



vertex_output VS(uint index: SV_VertexID)
{
    //	matrix m = { {1,0,0,100}, { 0,1,0,0 }, { 0,0,1,0 }, {0,0,0,1 } };

    matrix m = sceneData.GetNodes()[meshInfo.GetNode_offset()].GetNode_global_matrix();
    return transform(m, frameInfo.GetCamera(), meshData.GetVb()[meshInfo.GetVertex_offset()+ index]);
}

