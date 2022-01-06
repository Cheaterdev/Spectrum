#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherMeshesBoxes.h"
ConstantBuffer<GatherMeshesBoxes> pass_GatherMeshesBoxes: register( b2, space5);
const GatherMeshesBoxes CreateGatherMeshesBoxes()
{
	return pass_GatherMeshesBoxes;
}
#ifndef NO_GLOBAL
static const GatherMeshesBoxes gatherMeshesBoxes_global = CreateGatherMeshesBoxes();
const GatherMeshesBoxes GetGatherMeshesBoxes(){ return gatherMeshesBoxes_global; }
#endif
