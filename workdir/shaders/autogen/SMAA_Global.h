#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Global.h"
ConstantBuffer<SMAA_Global_cb> cb_2_0:register(b0,space2);
Texture2D srv_2_0: register(t0, space2);
SMAA_Global CreateSMAA_Global()
{
	SMAA_Global result;
	result.cb = cb_2_0;
	result.srv.colorTex = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Global sMAA_Global_global = CreateSMAA_Global();
const SMAA_Global GetSMAA_Global(){ return sMAA_Global_global; }
#endif
