#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyFace.h"
ConstantBuffer<SkyFace_cb> cb_4_0:register(b0,space4);
SkyFace CreateSkyFace()
{
	SkyFace result;
	result.cb = cb_4_0;
	return result;
}
static const SkyFace skyFace_global = CreateSkyFace();
const SkyFace GetSkyFace(){ return skyFace_global; }
