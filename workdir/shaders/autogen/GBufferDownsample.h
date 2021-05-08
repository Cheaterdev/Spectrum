#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float> srv_5_1: register(t1, space5);
struct Pass_GBufferDownsample
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_GBufferDownsample> pass_GBufferDownsample: register( b2, space5);
const GBufferDownsample CreateGBufferDownsample()
{
	GBufferDownsample result;
	Pass_GBufferDownsample pass;
	result.srv.normals = srv_5_0;
	result.srv.depth = srv_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
#endif
