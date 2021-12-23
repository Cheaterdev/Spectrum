#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawBoxes.h"
StructuredBuffer<float4> srv_5_0: register(t0, space5);
StructuredBuffer<BoxInfo> srv_5_1: register(t1, space5);
RWStructuredBuffer<uint> uav_5_0: register(u0, space5);
struct Pass_DrawBoxes
{
uint srv_0;
uint srv_1;
uint uav_0;
};
ConstantBuffer<Pass_DrawBoxes> pass_DrawBoxes: register( b2, space5);
const DrawBoxes CreateDrawBoxes()
{
	DrawBoxes result;
	result.srv.vertices = (pass_DrawBoxes.srv_0 );
	result.srv.input_meshes = (pass_DrawBoxes.srv_1 );
	result.uav.visible_meshes = (pass_DrawBoxes.uav_0 );
	return result;
}
#ifndef NO_GLOBAL
static const DrawBoxes drawBoxes_global = CreateDrawBoxes();
const DrawBoxes GetDrawBoxes(){ return drawBoxes_global; }
#endif
