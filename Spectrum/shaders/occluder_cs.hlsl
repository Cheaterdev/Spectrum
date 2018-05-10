struct draw_info
{
	uint data[60/4];
};

struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};

struct invisible_draw_info
{
   float3 mn;
   float3 mx;
    unsigned int instance_id;
  
   unsigned int pipeline_id;
   
   node_data node;
   
};

#define LIST \
X(0)\
X(1)\
X(2)\
X(3)\
X(4)\
X(5)\
X(6)\
X(7)\
X(8)\
X(9)\
X(10)\
X(11)\
X(12)\
X(13)\
X(14)\
X(15)

StructuredBuffer<uint> visibility_buffer: register(t0);
StructuredBuffer<draw_info> cpu_all_commands:register(t1);

#define X(x) AppendStructuredBuffer<draw_info> render_list_##x:register(u##x);
LIST
#undef X
//StructuredBuffer<invisible_draw_info> invisible_buffer: register(t0,space1);
#ifdef ALL_GOOD
StructuredBuffer<invisible_draw_info> boxes_instances: register(t0, space1);
#else
StructuredBuffer<uint> visible_ids: register(t0, space1);
#endif

cbuffer info : register(b0)
{
    uint _count;
	uint pipeline_id; 
	uint _per_thread;
};


[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{

	uint count=_count; 
	uint per_thread= _per_thread;


uint start_index = per_thread*(groupThreadID.x+64*groupID.x);
uint end_index = min(count,start_index+per_thread);
	
for( uint index=start_index;index<end_index;index++)
{
#ifdef ALL_GOOD
	const uint instance_id = index;
#else
	const uint instance_id = visible_ids[index];// clamp(visible_ids[index], 0, 100);
#endif

#ifdef OVERRIDED_MATERIAL
	uint pipeline = visibility_buffer[instance_id] - pipeline_id;
#else
	uint pipeline = visibility_buffer[instance_id] - pipeline_id;
#endif

	draw_info info = cpu_all_commands[instance_id];

#ifdef ALL_GOOD
	pipeline = boxes_instances[instance_id].pipeline_id - pipeline_id;
#endif
#define X(x) case x:{render_list_##x.Append(info); break;}
switch(pipeline)
	{
	LIST
	};
#undef X

	
}

}
