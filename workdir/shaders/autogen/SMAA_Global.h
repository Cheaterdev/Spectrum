#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Global.h"
ConstantBuffer<SMAA_Global_cb> cb_4_0:register(b0,space4);
Texture2D<float4> srv_4_0: register(t0, space4);
struct Pass_SMAA_Global
{
uint srv_0;
};
ConstantBuffer<Pass_SMAA_Global> pass_SMAA_Global: register( b2, space4);
const SMAA_Global CreateSMAA_Global()
{
	SMAA_Global result;
	result.cb = cb_4_0;
	result.srv.colorTex = (pass_SMAA_Global.srv_0 );
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Global sMAA_Global_global = CreateSMAA_Global();
const SMAA_Global GetSMAA_Global(){ return sMAA_Global_global; }
#endif
