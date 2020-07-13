#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherMeshesBoxes.h"
StructuredBuffer<BoxInfo> srv_4_0: register(t0, space4);
StructuredBuffer<uint> srv_4_1: register(t1, space4);
AppendStructuredBuffer<uint> uav_4_0: register(u0, space4);
AppendStructuredBuffer<uint> uav_4_1: register(u1, space4);
GatherMeshesBoxes CreateGatherMeshesBoxes()
{
	GatherMeshesBoxes result;
	result.srv.input_meshes = srv_4_0;
	result.srv.visible_boxes = srv_4_1;
	result.uav.visibleMeshes = uav_4_0;
	result.uav.invisibleMeshes = uav_4_1;
	return result;
}
static const GatherMeshesBoxes gatherMeshesBoxes_global = CreateGatherMeshesBoxes();
const GatherMeshesBoxes GetGatherMeshesBoxes(){ return gatherMeshesBoxes_global; }
