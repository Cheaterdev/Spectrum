#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/RaytracingRays.h"
ConstantBuffer<RaytracingRays> pass_RaytracingRays: register( b2, space6);
const RaytracingRays CreateRaytracingRays()
{
	return pass_RaytracingRays;
}
#ifndef NO_GLOBAL
static const RaytracingRays raytracingRays_global = CreateRaytracingRays();
const RaytracingRays GetRaytracingRays(){ return raytracingRays_global; }
#endif
