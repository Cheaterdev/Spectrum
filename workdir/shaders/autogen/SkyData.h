#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyData.h"
ConstantBuffer<SkyData_cb> cb_2_0:register(b0,space2);
Texture2D srv_2_0: register(t0, space2);
Texture2D srv_2_1: register(t1, space2);
Texture3D srv_2_2: register(t2, space2);
Texture2D srv_2_3: register(t3, space2);
SkyData CreateSkyData()
{
	SkyData result;
	result.cb = cb_2_0;
	result.srv.depthBuffer = srv_2_0;
	result.srv.transmittance = srv_2_1;
	result.srv.inscatter = srv_2_2;
	result.srv.irradiance = srv_2_3;
	return result;
}
#ifndef NO_GLOBAL
static const SkyData skyData_global = CreateSkyData();
const SkyData GetSkyData(){ return skyData_global; }
#endif
