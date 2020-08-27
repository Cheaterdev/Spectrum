#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
Texture2D srv_4_0: register(t0, space4);
Texture2D srv_4_1: register(t1, space4);
GBufferDownsample CreateGBufferDownsample()
{
	GBufferDownsample result;
	result.srv.normals = srv_4_0;
	result.srv.depth = srv_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
#endif
