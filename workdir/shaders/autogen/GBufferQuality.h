#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferQuality.h"
Texture2D srv_4_0: register(t0, space4);
GBufferQuality CreateGBufferQuality()
{
	GBufferQuality result;
	result.srv.ref = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferQuality gBufferQuality_global = CreateGBufferQuality();
const GBufferQuality GetGBufferQuality(){ return gBufferQuality_global; }
#endif
