#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
AppendStructuredBuffer<BoxInfo> uav_3_0: register(u0, space3);
AppendStructuredBuffer<uint> uav_3_1: register(u1, space3);
GatherBoxes CreateGatherBoxes()
{
	GatherBoxes result;
	result.uav.culledMeshes = uav_3_0;
	result.uav.visibleMeshes = uav_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
#endif
