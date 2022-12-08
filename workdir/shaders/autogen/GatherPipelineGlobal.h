#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GatherPipelineGlobal: register( b2, space4);
 ConstantBuffer<GatherPipelineGlobal> CreateGatherPipelineGlobal()
{
	return ResourceDescriptorHeap[pass_GatherPipelineGlobal.offset];
}
#ifndef NO_GLOBAL
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
#endif
