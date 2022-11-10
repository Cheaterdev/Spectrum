#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipeline.h"
ConstantBuffer<GatherPipeline> pass_GatherPipeline: register( b2, space5);
const GatherPipeline CreateGatherPipeline()
{
	return pass_GatherPipeline;
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
