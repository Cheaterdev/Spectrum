#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserHistoryFix.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float> srv_6_1: register(t1, space6);
RWTexture2D<float4> uav_6_0: register(u0, space6);
struct Pass_DenoiserHistoryFix
{
uint srv_0;
uint srv_1;
uint uav_0;
};
ConstantBuffer<Pass_DenoiserHistoryFix> pass_DenoiserHistoryFix: register( b2, space6);
const DenoiserHistoryFix CreateDenoiserHistoryFix()
{
	DenoiserHistoryFix result;
	Pass_DenoiserHistoryFix pass;
	result.srv.color = srv_6_0;
	result.srv.frames = srv_6_1;
	result.uav.target = uav_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const DenoiserHistoryFix denoiserHistoryFix_global = CreateDenoiserHistoryFix();
const DenoiserHistoryFix GetDenoiserHistoryFix(){ return denoiserHistoryFix_global; }
#endif
