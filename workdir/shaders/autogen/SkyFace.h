#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyFace.h"
ConstantBuffer<SkyFace_cb> cb_3_0:register(b0,space3);
SkyFace CreateSkyFace()
{
	SkyFace result;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const SkyFace skyFace_global = CreateSkyFace();
const SkyFace GetSkyFace(){ return skyFace_global; }
#endif
