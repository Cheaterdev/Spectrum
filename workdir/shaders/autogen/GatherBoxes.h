#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
AppendStructuredBuffer<BoxInfo> uav_4_0: register(u0, space4);
AppendStructuredBuffer<uint> uav_4_1: register(u1, space4);
GatherBoxes CreateGatherBoxes()
{
	GatherBoxes result;
	result.uav.culledMeshes = uav_4_0;
	result.uav.visibleMeshes = uav_4_1;
	return result;
}
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
