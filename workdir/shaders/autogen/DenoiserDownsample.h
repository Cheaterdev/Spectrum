#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserDownsample.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float> srv_6_1: register(t1, space6);
struct Pass_DenoiserDownsample
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_DenoiserDownsample> pass_DenoiserDownsample: register( b2, space6);
const DenoiserDownsample CreateDenoiserDownsample()
{
	DenoiserDownsample result;
	result.srv.color = (pass_DenoiserDownsample.srv_0 );
	result.srv.depth = (pass_DenoiserDownsample.srv_1 );
	return result;
}
#ifndef NO_GLOBAL
static const DenoiserDownsample denoiserDownsample_global = CreateDenoiserDownsample();
const DenoiserDownsample GetDenoiserDownsample(){ return denoiserDownsample_global; }
#endif
