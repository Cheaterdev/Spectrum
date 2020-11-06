#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawBoxes.h"
StructuredBuffer<float4> srv_4_0: register(t0, space4);
StructuredBuffer<BoxInfo> srv_4_1: register(t1, space4);
RWStructuredBuffer<uint> uav_4_0: register(u0, space4);
DrawBoxes CreateDrawBoxes()
{
	DrawBoxes result;
	result.srv.vertices = srv_4_0;
	result.srv.input_meshes = srv_4_1;
	result.uav.visible_meshes = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const DrawBoxes drawBoxes_global = CreateDrawBoxes();
const DrawBoxes GetDrawBoxes(){ return drawBoxes_global; }
#endif
