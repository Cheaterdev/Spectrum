
struct vertex_input
{
float3 pos : POSITION;
float3 normal : NORMAL;
float2 tc : TEXCOORD;
float4 tangent : TANGENT;
};

struct vertex_output
{
float4 pos : SV_POSITION;
};

struct camera_info
{
      matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_proj;
    matrix inv_view_proj;
    float3 position;
    float unused;
    float3 direction;
    float unused2;
};

struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};

 
cbuffer cbCamera : register(b0)
{
    camera_info camera;
};

/*
cbuffer cbMesh : register(b1)
{
   node_data node;
};*/


cbuffer cbMaterials : register(b2)
{
	uint texture_offset;
	uint node_offset;
};

vertex_output transform(camera_info camera, node_data node, vertex_input i)
{
    vertex_output o;
    float4 tpos =  mul(node.node_global_matrix, float4(i.pos.xyz, 1));
    o.pos = mul(camera.view_proj,  tpos);
  
    return o;
}


StructuredBuffer<vertex_input> vb: register(t0);
StructuredBuffer<node_data> nodes: register(t1, space1);

vertex_output VS(uint index: SV_VertexID)
{
	//instance_data data = instances[instance];
	vertex_input input = vb[index];
	//input.pos.xyz=float4(input.pos.xyz/2+0.5)*(data.mx-data.mn);

	vertex_output o = transform(camera, nodes[node_offset],input);;

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


cbuffer cbPos : register(b2)
{
    float3 offset;
};



vertex_output VS_COLOR(uint index: SV_VertexID)
{
	vertex_input input = vb[index];

	vertex_output o = transform(camera, nodes[node_offset],input);;
    o.pos = mul(camera.view_proj,  float4(input.pos.xyz+offset,1));
    return o;
}



cbuffer cbColor : register(b0)
{
    float3 color;
};


float4 PS_COLOR(vertex_output i):SV_TARGET0
{
return float4(color,1);
}