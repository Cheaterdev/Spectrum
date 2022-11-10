#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
ConstantBuffer<GatherPipelineGlobal> pass_GatherPipelineGlobal: register( b2, space4);
const GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	return pass_GatherPipelineGlobal;
}
#ifndef NO_GLOBAL
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
#endif
