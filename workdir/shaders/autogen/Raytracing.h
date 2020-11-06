#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Raytracing.h"
RaytracingAccelerationStructure srv_6_0: register(t0, space6);
StructuredBuffer<uint> srv_6_1: register(t1, space6);
Raytracing CreateRaytracing()
{
	Raytracing result;
	result.srv.scene = srv_6_0;
	result.srv.index_buffer = srv_6_1;
	return result;
}
#ifndef NO_GLOBAL
static const Raytracing raytracing_global = CreateRaytracing();
const Raytracing GetRaytracing(){ return raytracing_global; }
#endif
