#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Raytracing.h"
RaytracingAccelerationStructure srv_5_0: register(t0, space5);
StructuredBuffer<uint> srv_5_1: register(t1, space5);
Raytracing CreateRaytracing()
{
	Raytracing result;
	result.srv.scene = srv_5_0;
	result.srv.index_buffer = srv_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const Raytracing raytracing_global = CreateRaytracing();
const Raytracing GetRaytracing(){ return raytracing_global; }
#endif
