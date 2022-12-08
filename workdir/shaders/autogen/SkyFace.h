#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyFace.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SkyFace: register( b2, space5);
 ConstantBuffer<SkyFace> CreateSkyFace()
{
	return ResourceDescriptorHeap[pass_SkyFace.offset];
}
#ifndef NO_GLOBAL
static const SkyFace skyFace_global = CreateSkyFace();
const SkyFace GetSkyFace(){ return skyFace_global; }
#endif
