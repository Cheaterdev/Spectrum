#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
Texture2D srv_5_0: register(t0, space5);
Texture2D srv_5_1: register(t1, space5);
GBufferDownsample CreateGBufferDownsample()
{
	GBufferDownsample result;
	result.srv.normals = srv_5_0;
	result.srv.depth = srv_5_1;
	return result;
}
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
