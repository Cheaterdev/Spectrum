
struct node_data
{
    matrix node_global_matrix;
	matrix node_inverse_matrix;
};

struct draw_info
{
   float3 mn;
   float3 mx;
    unsigned int instance_id;
  
   unsigned int pipeline_id;
   
   node_data node;
   
};


struct dispatch_info
{
 
    unsigned int count;
  uint3 dispatch_count;
   
};


StructuredBuffer<uint> view_buffer: register(t0);

StructuredBuffer<uint> all_visible_ids:register(t0, space1);
StructuredBuffer<draw_info> all_list:register(t1);
AppendStructuredBuffer<draw_info> render_list:register(u0);

RWStructuredBuffer<uint> visible_list: register(u1, space1);

groupshared uint max_count=0;

[numthreads(64, 1, 1)]
void CS(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID    : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex    : SV_GroupIndex
)
{

	uint count; uint stride;

	all_visible_ids.GetDimensions(count, stride);

uint per_thread = max(1, count/64);
uint tid = groupThreadID.x+64*groupID.x;
uint start_index = per_thread*(tid);
uint end_index = min(count,start_index+per_thread);

int my_count = 0;

for( uint index=start_index;index<end_index;index++)
{
	uint global_index = all_visible_ids[index];
	if (global_index == -1) continue;
	draw_info info = all_list[global_index];
	bool invisible = (view_buffer[info.instance_id] == 0);
	if(invisible){ 
		uint counter = visible_list.IncrementCounter();
		visible_list[counter] = info.instance_id;

		render_list.Append(info);
		my_count++;

	}
}

/*
InterlockedAdd(max_count, my_count);

	GroupMemoryBarrierWithGroupSync();

if(tid==0)
{
//max_count
dispatch_info dinfo;
dinfo.count = max_count;
dinfo.dispatch_count=uint3(((max_count + 64 *32 - 1) / (64 *32)),1,1);
dispach_buffer[0]=dinfo;
}*/
}



RWStructuredBuffer<dispatch_info> dispach_buffer:register(u0, space1);

[numthreads(1, 1, 1)]
void CS_Dispatch(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex
)
{
	uint max_count = visible_list.IncrementCounter();
		//max_count
		dispatch_info dinfo;
		dinfo.count = max_count;
		dinfo.dispatch_count = uint3(((max_count + 64 * 32 - 1) / (64 * 32)), 1, 1);
		dispach_buffer[0] = dinfo;
	
}


