#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherMeshesBoxes.h"
StructuredBuffer<BoxInfo> srv_5_0: register(t0, space5);
StructuredBuffer<uint> srv_5_1: register(t1, space5);
AppendStructuredBuffer<uint> uav_5_0: register(u0, space5);
AppendStructuredBuffer<uint> uav_5_1: register(u1, space5);
struct Pass_GatherMeshesBoxes
{
uint srv_0;
uint srv_1;
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_GatherMeshesBoxes> pass_GatherMeshesBoxes: register( b2, space5);
const GatherMeshesBoxes CreateGatherMeshesBoxes()
{
	GatherMeshesBoxes result;
	Pass_GatherMeshesBoxes pass;
	result.srv.input_meshes = srv_5_0;
	result.srv.visible_boxes = srv_5_1;
	result.uav.visibleMeshes = uav_5_0;
	result.uav.invisibleMeshes = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherMeshesBoxes gatherMeshesBoxes_global = CreateGatherMeshesBoxes();
const GatherMeshesBoxes GetGatherMeshesBoxes(){ return gatherMeshesBoxes_global; }
#endif
