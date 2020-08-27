#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherMeshesBoxes.h"
StructuredBuffer<BoxInfo> srv_3_0: register(t0, space3);
StructuredBuffer<uint> srv_3_1: register(t1, space3);
AppendStructuredBuffer<uint> uav_3_0: register(u0, space3);
AppendStructuredBuffer<uint> uav_3_1: register(u1, space3);
GatherMeshesBoxes CreateGatherMeshesBoxes()
{
	GatherMeshesBoxes result;
	result.srv.input_meshes = srv_3_0;
	result.srv.visible_boxes = srv_3_1;
	result.uav.visibleMeshes = uav_3_0;
	result.uav.invisibleMeshes = uav_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherMeshesBoxes gatherMeshesBoxes_global = CreateGatherMeshesBoxes();
const GatherMeshesBoxes GetGatherMeshesBoxes(){ return gatherMeshesBoxes_global; }
#endif
