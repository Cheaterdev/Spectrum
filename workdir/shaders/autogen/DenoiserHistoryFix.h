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
struct Pass_DenoiserHistoryFix
{
uint srv_0;
uint srv_1;
uint srv_2;
uint uav_0;
};
ConstantBuffer<Pass_DenoiserHistoryFix> pass_DenoiserHistoryFix: register( b2, space5);
const DenoiserHistoryFix CreateDenoiserHistoryFix()
{
	DenoiserHistoryFix result;
	Pass_DenoiserHistoryFix pass;
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
