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
	result.uav.commands = uav_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
