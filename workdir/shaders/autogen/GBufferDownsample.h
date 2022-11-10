#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
ConstantBuffer<GBufferDownsample> pass_GBufferDownsample: register( b2, space6);
const GBufferDownsample CreateGBufferDownsample()
{
	return pass_GBufferDownsample;
}
#ifndef NO_GLOBAL
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
#endif
