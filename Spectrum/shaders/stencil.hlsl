#include "Common.hlsl"
#include "Common2D.hlsl"

struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};

 
cbuffer cbCamera : register(b0)
{
    camera_info camera;
};


cbuffer cbMaterials : register(b2)
{
	uint texture_offset;
	uint node_offset;
};


StructuredBuffer<vertex_input> vb: register(t0);
StructuredBuffer<node_data> nodes: register(t1, space1);

vertex_output VS(uint index: SV_VertexID)
{
    vertex_input input = vb[index];
	vertex_output o = transform(camera, nodes[node_offset].node_global_matrix ,input);
    return o;
}



cbuffer cbInstance : register(b0)
{
      unsigned int instance_id;
    
};

RWStructuredBuffer<uint> view_buffer: register(u0);

[earlydepthstencil]
void PS(vertex_output i)
{
	view_buffer[0]=instance_id;
}

float4 PS_RESULT(vertex_output i):SV_TARGET0
{
return 1;
}


/*
cbuffer cbPos : register(b2)
{
    float3 offset;
};
*/


vertex_output VS_COLOR(uint index: SV_VertexID)
{
	vertex_input input = vb[index];

	vertex_output o = transform(camera, nodes[node_offset].node_global_matrix,input);;
  //  o.pos = mul(camera.view_proj,  float4(input.pos.xyz+offset,1));
    return o;
}


cbuffer cbColor : register(b0)
{
    float3 color;
};
float4 PS_COLOR(vertex_output i) :SV_TARGET0
{
return float4(color,1);
}
