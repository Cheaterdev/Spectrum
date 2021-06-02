#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyData.h"
ConstantBuffer<SkyData_cb> cb_4_0:register(b0,space4);
Texture2D<float> srv_4_0: register(t0, space4);
Texture2D<float4> srv_4_1: register(t1, space4);
Texture3D<float4> srv_4_2: register(t2, space4);
Texture2D<float4> srv_4_3: register(t3, space4);
struct Pass_SkyData
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
};
ConstantBuffer<Pass_SkyData> pass_SkyData: register( b2, space4);
const SkyData CreateSkyData()
{
	SkyData result;
	result.cb = cb_4_0;
	result.srv.depthBuffer = srv_4_0;
	result.srv.transmittance = srv_4_1;
	result.srv.inscatter = srv_4_2;
	result.srv.irradiance = srv_4_3;
	return result;
}
#ifndef NO_GLOBAL
static const SkyData skyData_global = CreateSkyData();
const SkyData GetSkyData(){ return skyData_global; }
#endif
