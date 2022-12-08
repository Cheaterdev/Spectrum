#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherMeshesBoxes.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GatherMeshesBoxes: register( b2, space5);
 ConstantBuffer<GatherMeshesBoxes> CreateGatherMeshesBoxes()
{
	return ResourceDescriptorHeap[pass_GatherMeshesBoxes.offset];
}
#ifndef NO_GLOBAL
static const GatherMeshesBoxes gatherMeshesBoxes_global = CreateGatherMeshesBoxes();
const GatherMeshesBoxes GetGatherMeshesBoxes(){ return gatherMeshesBoxes_global; }
#endif
