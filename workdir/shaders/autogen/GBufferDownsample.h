#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float> srv_6_1: register(t1, space6);
struct Pass_GBufferDownsample
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_GBufferDownsample> pass_GBufferDownsample: register( b2, space6);
const GBufferDownsample CreateGBufferDownsample()
{
	GBufferDownsample result;
	result.srv.normals = srv_6_0;
	result.srv.depth = srv_6_1;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
#endif
