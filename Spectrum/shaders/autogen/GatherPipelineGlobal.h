#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
ConstantBuffer<GatherPipelineGlobal_cb> cb_3_0:register(b0,space3);
Buffer<uint> srv_3_0: register(t0, space3);
GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	GatherPipelineGlobal result;
	result.cb = cb_3_0;
	result.srv.commands = srv_3_0;
	return result;
}
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
