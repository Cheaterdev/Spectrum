#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserDownsample.h"
ConstantBuffer<DenoiserDownsample> pass_DenoiserDownsample: register( b2, space6);
const DenoiserDownsample CreateDenoiserDownsample()
{
	return pass_DenoiserDownsample;
}
#ifndef NO_GLOBAL
static const DenoiserDownsample denoiserDownsample_global = CreateDenoiserDownsample();
const DenoiserDownsample GetDenoiserDownsample(){ return denoiserDownsample_global; }
#endif
