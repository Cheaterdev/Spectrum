
struct dispatch_info
{
 
    unsigned int count;
  uint3 dispatch_count;
   
};

RWStructuredBuffer<uint> visible_list: register(u0);
RWStructuredBuffer<dispatch_info> dispach_buffer:register(u1);

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


