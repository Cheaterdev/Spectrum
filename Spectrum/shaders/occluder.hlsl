
struct vertex_input
{
float4 pos : POSITION;
};

struct vertex_output
{
float4 pos : SV_POSITION;
unsigned int instance:INSTANCE_ID;
  unsigned int pipeline_id:PIPELINE_ID;
  
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
    float3 direction;
};

struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};

struct instance_data
{
   float3 mn;
   float3 mx;
     unsigned int instance_id;
  
   unsigned int pipeline_id;
   
   node_data node;
   
};



 
cbuffer cbCamera : register(b0)
{
    camera_info camera;
};

vertex_output transform(camera_info camera, node_data node, vertex_input i)
{
    vertex_output o;
    float4 tpos =  mul(node.node_global_matrix, float4(i.pos.xyz, 1));
    o.pos = mul(camera.view_proj, tpos);
  
    return o;
}


StructuredBuffer<vertex_input> vb: register(t0, space1);
//StructuredBuffer<node_data> nodes: register(t1, space1);
StructuredBuffer<instance_data> instances: register(t2, space1);

 
vertex_output VS(uint index: SV_VertexID,uint instance: SV_InstanceID)
{
	instance_data data = instances[instance];
	vertex_input input = vb[index];
	input.pos.xyz=data.mn+(input.pos.xyz/2+0.5)*(data.mx-data.mn);
	
	vertex_output o = transform(camera, data.node,input);;
	o.instance = data.instance_id;
	o.pipeline_id = data.pipeline_id;
	
    return o;
}

RWStructuredBuffer<uint> view_buffer: register(u0);

[earlydepthstencil]
void PS(vertex_output i)
{
	view_buffer[i.instance]=i.pipeline_id ;
}
