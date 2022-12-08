#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/RaytracingRays.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_RaytracingRays: register( b2, space6);
 ConstantBuffer<RaytracingRays> CreateRaytracingRays()
{
	return ResourceDescriptorHeap[pass_RaytracingRays.offset];
}
#ifndef NO_GLOBAL
static const RaytracingRays raytracingRays_global = CreateRaytracingRays();
const RaytracingRays GetRaytracingRays(){ return raytracingRays_global; }
#endif
