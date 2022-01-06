#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferQuality.h"
ConstantBuffer<GBufferQuality> pass_GBufferQuality: register( b2, space6);
const GBufferQuality CreateGBufferQuality()
{
	return pass_GBufferQuality;
}
#ifndef NO_GLOBAL
static const GBufferQuality gBufferQuality_global = CreateGBufferQuality();
const GBufferQuality GetGBufferQuality(){ return gBufferQuality_global; }
#endif
