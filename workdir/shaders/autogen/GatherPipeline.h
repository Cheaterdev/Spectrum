#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipeline.h"
ConstantBuffer<GatherPipeline_cb> cb_5_0:register(b0,space5);
AppendStructuredBuffer<CommandData> uav_5_0[8]: register(u0, space5);
struct Pass_GatherPipeline
{
uint uav_0;
};
ConstantBuffer<Pass_GatherPipeline> pass_GatherPipeline: register( b2, space5);
const GatherPipeline CreateGatherPipeline()
{
	GatherPipeline result;
	result.cb = cb_5_0;
	result.uav.commands[0] = (pass_GatherPipeline.uav_0) + 0; 
	result.uav.commands[1] = (pass_GatherPipeline.uav_0) + 1; 
	result.uav.commands[2] = (pass_GatherPipeline.uav_0) + 2; 
	result.uav.commands[3] = (pass_GatherPipeline.uav_0) + 3; 
	result.uav.commands[4] = (pass_GatherPipeline.uav_0) + 4; 
	result.uav.commands[5] = (pass_GatherPipeline.uav_0) + 5; 
	result.uav.commands[6] = (pass_GatherPipeline.uav_0) + 6; 
	result.uav.commands[7] = (pass_GatherPipeline.uav_0) + 7; 
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
