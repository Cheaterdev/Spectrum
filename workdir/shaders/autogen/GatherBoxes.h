#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
AppendStructuredBuffer<BoxInfo> uav_5_0: register(u0, space5);
AppendStructuredBuffer<uint> uav_5_1: register(u1, space5);
struct Pass_GatherBoxes
{
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_GatherBoxes> pass_GatherBoxes: register( b2, space5);
const GatherBoxes CreateGatherBoxes()
{
	GatherBoxes result;
	result.uav.culledMeshes = uav_5_0;
	result.uav.visibleMeshes = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
#endif
