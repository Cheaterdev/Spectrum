#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyFace.h"
ConstantBuffer<SkyFace_cb> cb_5_0:register(b0,space5);
struct Pass_SkyFace
{
};
ConstantBuffer<Pass_SkyFace> pass_SkyFace: register( b2, space5);
const SkyFace CreateSkyFace()
{
	SkyFace result;
	Pass_SkyFace pass;
	result.cb = cb_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const SkyFace skyFace_global = CreateSkyFace();
const SkyFace GetSkyFace(){ return skyFace_global; }
#endif
