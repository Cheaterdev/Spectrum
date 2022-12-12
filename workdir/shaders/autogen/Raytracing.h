#ifndef SLOT_10
	#define SLOT_10
#else
	#error Slot 10 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Raytracing.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Raytracing: register( b2, space10);
 ConstantBuffer<Raytracing> CreateRaytracing()
{
	return ResourceDescriptorHeap[pass_Raytracing.offset];
}
#ifndef NO_GLOBAL
static const Raytracing raytracing_global = CreateRaytracing();
const Raytracing GetRaytracing(){ return raytracing_global; }
#endif
