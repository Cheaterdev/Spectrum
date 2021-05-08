#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserDownsample.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float> srv_5_1: register(t1, space5);
struct Pass_DenoiserDownsample
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_DenoiserDownsample> pass_DenoiserDownsample: register( b2, space5);
const DenoiserDownsample CreateDenoiserDownsample()
{
	DenoiserDownsample result;
	Pass_DenoiserDownsample pass;
	result.srv.color = srv_5_0;
	result.srv.depth = srv_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const DenoiserDownsample denoiserDownsample_global = CreateDenoiserDownsample();
const DenoiserDownsample GetDenoiserDownsample(){ return denoiserDownsample_global; }
#endif
