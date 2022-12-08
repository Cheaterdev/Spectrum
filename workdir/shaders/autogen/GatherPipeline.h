#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipeline.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GatherPipeline: register( b2, space5);
 ConstantBuffer<GatherPipeline> CreateGatherPipeline()
{
	return ResourceDescriptorHeap[pass_GatherPipeline.offset];
}
#ifndef NO_GLOBAL
static const GatherPipeline gatherPipeline_global = CreateGatherPipeline();
const GatherPipeline GetGatherPipeline(){ return gatherPipeline_global; }
#endif
