#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Global.h"
ConstantBuffer<SMAA_Global_cb> cb_3_0:register(b0,space3);
Texture2D<float4> srv_3_0: register(t0, space3);
struct Pass_SMAA_Global
{
uint srv_0;
};
ConstantBuffer<Pass_SMAA_Global> pass_SMAA_Global: register( b2, space3);
const SMAA_Global CreateSMAA_Global()
{
	SMAA_Global result;
	Pass_SMAA_Global pass;
	result.cb = cb_3_0;
	result.srv.colorTex = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Global sMAA_Global_global = CreateSMAA_Global();
const SMAA_Global GetSMAA_Global(){ return sMAA_Global_global; }
#endif
