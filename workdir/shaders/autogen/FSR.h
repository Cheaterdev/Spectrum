#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FSR.h"
ConstantBuffer<FSR_cb> cb_4_0:register(b0,space4);
Texture2D<float4> srv_4_0: register(t0, space4);
RWTexture2D<float4> uav_4_0: register(u0, space4);
struct Pass_FSR
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_FSR> pass_FSR: register( b2, space4);
const FSR CreateFSR()
{
	FSR result;
	result.cb = cb_4_0;
	result.srv.source = srv_4_0;
	result.uav.target = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const FSR fSR_global = CreateFSR();
const FSR GetFSR(){ return fSR_global; }
#endif
