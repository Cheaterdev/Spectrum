#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyFace.h"
ConstantBuffer<SkyFace> pass_SkyFace: register( b2, space5);
const SkyFace CreateSkyFace()
{
	return pass_SkyFace;
}
#ifndef NO_GLOBAL
static const SkyFace skyFace_global = CreateSkyFace();
const SkyFace GetSkyFace(){ return skyFace_global; }
#endif
