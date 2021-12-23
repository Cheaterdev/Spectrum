#ifndef SLOT_7
	#define SLOT_7
#else
	#error Slot 7 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Raytracing.h"
RaytracingAccelerationStructure srv_7_0: register(t0, space7);
StructuredBuffer<uint> srv_7_1: register(t1, space7);
struct Pass_Raytracing
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_Raytracing> pass_Raytracing: register( b2, space7);
const Raytracing CreateRaytracing()
{
	Raytracing result;
	result.srv.scene = (pass_Raytracing.srv_0 );
	result.srv.index_buffer = (pass_Raytracing.srv_1 );
	return result;
}
#ifndef NO_GLOBAL
static const Raytracing raytracing_global = CreateRaytracing();
const Raytracing GetRaytracing(){ return raytracing_global; }
#endif
