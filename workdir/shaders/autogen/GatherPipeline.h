#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipeline.h"
ConstantBuffer<GatherPipeline_cb> cb_4_0:register(b0,space4);
AppendStructuredBuffer<CommandData> uav_4_0[8]: register(u0, space4);
GatherPipeline CreateGatherPipeline()
{
	GatherPipeline result;
	result.cb = cb_4_0;
	result.uav.commands = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
