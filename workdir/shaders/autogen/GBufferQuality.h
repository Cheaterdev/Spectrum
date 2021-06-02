#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferQuality.h"
Texture2D<float4> srv_6_0: register(t0, space6);
struct Pass_GBufferQuality
{
uint srv_0;
};
ConstantBuffer<Pass_GBufferQuality> pass_GBufferQuality: register( b2, space6);
const GBufferQuality CreateGBufferQuality()
{
	GBufferQuality result;
	result.srv.ref = srv_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const GBufferQuality gBufferQuality_global = CreateGBufferQuality();
const GBufferQuality GetGBufferQuality(){ return gBufferQuality_global; }
#endif
