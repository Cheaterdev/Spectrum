
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


/*
cbuffer cbCamera : register(b0)
{
    camera_info camera;
    camera_info prev_camera;
};


cbuffer cbMaterials : register(b2)
{
    uint texture_offset;
    uint node_offset;
};


*/
/*
cbuffer cbNode : register(b1)
{
    matrix node_global_matrix;
};*/

struct FrameInfo_constants
{
    camera_info camera;
    camera_info prev_camera;
};

struct Material_constants
{
    uint texture_offset;
    uint node_offset;
};

struct FrameInfo
{
    FrameInfo_constants constants;
    camera_info GetCamera() { return constants.camera; }
    camera_info GetPrevCamera() { return constants.prev_camera; }

    FrameInfo(FrameInfo_constants _constants)
    {
        constants = _constants;
    }
};

struct MaterialInfo
{
    Material_constants constants;

    camera_info Gettexture_offset() { return constants.texture_offset; }
    camera_info Getnode_offset() { return constants.node_offset; }
};

cbuffer FrameInfo_constants_cb : register(b0, space0)
{
    FrameInfo_constants constants;
};


cbuffer MaterialInfo_constants_cb : register(b2, space0)
{
    MaterialInfo_constants constants;
};

FrameInfo CreateFrameInfo()
{
    return FrameInfo(FrameInfo_constants_cb);
}

MaterialInfo CreateMaterialInfo()
{
    return  MaterialInfo(MaterialInfo_constants_cb);
}



static const frameInfo = CreateFrameInfo();
static const materialInfo = CreateMaterialInfo();

dfgfdgdfg
vertex_output transform(matrix node_global_matrix, camera_info camera, vertex_input i)
{
    vertex_output o;
    float4 tpos = mul(node_global_matrix, float4(i.pos, 1));
    o.wpos.xyz = tpos.xyz / tpos.w;
    o.pos = mul(frameInfo.GetCamera().view_proj, tpos);
	o.normal = normalize(mul((float3x3)node_global_matrix, i.normal));
    o.tangent = normalize(mul((float3x3) node_global_matrix, i.tangent.xyz) * i.tangent.w);
    o.binormal =   normalize(cross(i.normal, i.tangent.xyz) * i.tangent.w);
    o.tc = i.tc;
    float3 v = o.wpos.xyz - frameInfo.GetCamera().position;
    float dist = length(v);
    o.dist = (1 - clamp(dist, 10, 40) / 40);
    o.dist *= 1 + pow(1 - abs(dot(o.normal, v / dist)), 4);
    o.dist = clamp(o.dist, 0, 1);
    o.cur_pos = o.pos;
     
	o.prev_pos = mul(frameInfo.GetPrevCamera().view_proj, tpos);
    return o;
}
-

StructuredBuffer<vertex_input> vb: register(t0, space1);
StructuredBuffer<node_data> nodes: register(t1, space1);


vertex_output VS(uint index: SV_VertexID)
{
//	matrix m = { {1,0,0,100}, { 0,1,0,0 }, { 0,0,1,0 }, {0,0,0,1 } };

	matrix m = nodes[materialInfo.Getnode_offset()].node_global_matrix;
    return transform(m, camera, vb[index]);
}

 