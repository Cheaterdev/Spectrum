#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipeline.h"
ConstantBuffer<GatherPipeline_cb> cb_3_0:register(b0,space3);
AppendStructuredBuffer<CommandData> uav_3_0[8]: register(u0, space3);
GatherPipeline CreateGatherPipeline()
{
	GatherPipeline result;
	result.cb = cb_3_0;
	result.uav.commands = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
