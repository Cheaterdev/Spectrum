#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserHistoryFix.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float> srv_5_1: register(t1, space5);
RWTexture2D<float4> uav_5_0: register(u0, space5);
StructuredBuffer<uint2> srv_5_2: register(t2, space5);
DenoiserHistoryFix CreateDenoiserHistoryFix()
{
	DenoiserHistoryFix result;
	result.srv.color = srv_5_0;
	result.srv.frames = srv_5_1;
	result.uav.target = uav_5_0;
	result.srv.tiling.tiles = srv_5_2;
	return result;
}
#ifndef NO_GLOBAL
static const DenoiserHistoryFix denoiserHistoryFix_global = CreateDenoiserHistoryFix();
const DenoiserHistoryFix GetDenoiserHistoryFix(){ return denoiserHistoryFix_global; }
#endif
