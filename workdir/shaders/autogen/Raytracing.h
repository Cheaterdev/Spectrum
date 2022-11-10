#ifndef SLOT_7
	#define SLOT_7
#else
	#error Slot 7 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Raytracing.h"
ConstantBuffer<Raytracing> pass_Raytracing: register( b2, space7);
const Raytracing CreateRaytracing()
{
	return pass_Raytracing;
}
#ifndef NO_GLOBAL
static const Raytracing raytracing_global = CreateRaytracing();
const Raytracing GetRaytracing(){ return raytracing_global; }
#endif
