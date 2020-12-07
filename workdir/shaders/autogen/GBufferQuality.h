#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferQuality.h"
Texture2D<float4> srv_5_0: register(t0, space5);
GBufferQuality CreateGBufferQuality()
{
	GBufferQuality result;
	result.srv.ref = srv_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferQuality gBufferQuality_global = CreateGBufferQuality();
const GBufferQuality GetGBufferQuality(){ return gBufferQuality_global; }
#endif
