#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GatherBoxes: register( b2, space5);
 ConstantBuffer<GatherBoxes> CreateGatherBoxes()
{
	return ResourceDescriptorHeap[pass_GatherBoxes.offset];
}
#ifndef NO_GLOBAL
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
#endif
