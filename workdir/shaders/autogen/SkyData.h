#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyData.h"
ConstantBuffer<SkyData_cb> cb_3_0:register(b0,space3);
Texture2D srv_3_0: register(t0, space3);
Texture2D srv_3_1: register(t1, space3);
Texture3D srv_3_2: register(t2, space3);
Texture2D srv_3_3: register(t3, space3);
SkyData CreateSkyData()
{
	SkyData result;
	result.cb = cb_3_0;
	result.srv.depthBuffer = srv_3_0;
	result.srv.transmittance = srv_3_1;
	result.srv.inscatter = srv_3_2;
	result.srv.irradiance = srv_3_3;
	return result;
}
#ifndef NO_GLOBAL
static const SkyData skyData_global = CreateSkyData();
const SkyData GetSkyData(){ return skyData_global; }
#endif
