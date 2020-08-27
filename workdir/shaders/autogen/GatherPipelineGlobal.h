#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
ConstantBuffer<GatherPipelineGlobal_cb> cb_2_0:register(b0,space2);
Buffer<uint> srv_2_0: register(t0, space2);
GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	GatherPipelineGlobal result;
	result.cb = cb_2_0;
	result.srv.commands = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
#endif
