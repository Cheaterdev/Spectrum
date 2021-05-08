#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
AppendStructuredBuffer<BoxInfo> uav_4_0: register(u0, space4);
AppendStructuredBuffer<uint> uav_4_1: register(u1, space4);
struct Pass_GatherBoxes
{
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_GatherBoxes> pass_GatherBoxes: register( b2, space4);
const GatherBoxes CreateGatherBoxes()
{
	GatherBoxes result;
	Pass_GatherBoxes pass;
	result.uav.culledMeshes = uav_4_0;
	result.uav.visibleMeshes = uav_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
#endif
