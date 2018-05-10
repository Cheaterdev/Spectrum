
struct vertex_input
{
float3 pos : POSITION;
float3 normal : NORMAL;
float2 tc : TEXCOORD;
float4 tangent : TANGENT;
    //float3 binormal : BINORMAL;
};

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

struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};



cbuffer cbCamera : register(b0)
{
    camera_info camera;
    camera_info prev_camera;
};
/*
cbuffer cbNode : register(b1)
{
    matrix node_global_matrix;
};*/

cbuffer cbMaterials : register(b2)
{
	uint texture_offset;
	uint node_offset;
};


vertex_output transform(matrix node_global_matrix, camera_info camera, vertex_input i)
{
    vertex_output o;
    float4 tpos = mul(node_global_matrix, float4(i.pos, 1));
    o.wpos.xyz = tpos.xyz / tpos.w;
    o.pos = mul(camera.view_proj, tpos);
	o.normal = normalize(mul((float3x3)node_global_matrix, i.normal));
    o.tangent = normalize(mul((float3x3) node_global_matrix, i.tangent.xyz) * i.tangent.w);
    o.binormal =   normalize(cross(i.normal, i.tangent.xyz) * i.tangent.w);
    o.tc = i.tc;
    float3 v = o.wpos.xyz - camera.position;
    float dist = length(v);
    o.dist = (1 - clamp(dist, 10, 40) / 40);
    o.dist *= 1 + pow(1 - abs(dot(o.normal, v / dist)), 4);
    o.dist = clamp(o.dist, 0, 1);
    o.cur_pos = o.pos;
   
	o.prev_pos = mul(prev_camera.view_proj, tpos);
    return o;
}


StructuredBuffer<vertex_input> vb: register(t0, space1);
StructuredBuffer<node_data> nodes: register(t1, space1);


vertex_output VS(uint index: SV_VertexID)
{
//	matrix m = { {1,0,0,100}, { 0,1,0,0 }, { 0,0,1,0 }, {0,0,0,1 } };

	matrix m = nodes[node_offset].node_global_matrix;
    return transform(m, camera, vb[index]);
}

 