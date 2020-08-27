#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawBoxes.h"
StructuredBuffer<float4> srv_3_0: register(t0, space3);
StructuredBuffer<BoxInfo> srv_3_1: register(t1, space3);
RWStructuredBuffer<uint> uav_3_0: register(u0, space3);
DrawBoxes CreateDrawBoxes()
{
	DrawBoxes result;
	result.srv.vertices = srv_3_0;
	result.srv.input_meshes = srv_3_1;
	result.uav.visible_meshes = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const DrawBoxes drawBoxes_global = CreateDrawBoxes();
const DrawBoxes GetDrawBoxes(){ return drawBoxes_global; }
#endif
