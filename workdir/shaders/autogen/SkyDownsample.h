#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyDownsample.h"
ConstantBuffer<SkyDownsample_cb> cb_5_0:register(b0,space5);
TextureCube srv_5_0: register(t0, space5);
SkyDownsample CreateSkyDownsample()
{
	SkyDownsample result;
	result.cb = cb_5_0;
	result.srv.sourceTex = srv_5_0;
	return result;
}
static const SkyDownsample skyDownsample_global = CreateSkyDownsample();
const SkyDownsample GetSkyDownsample(){ return skyDownsample_global; }
