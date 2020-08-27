#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DownsampleDepth.h"
Texture2D<float> srv_2_0: register(t0, space2);
RWTexture2D<float> uav_2_0: register(u0, space2);
DownsampleDepth CreateDownsampleDepth()
{
	DownsampleDepth result;
	result.srv.srcTex = srv_2_0;
	result.uav.targetTex = uav_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const DownsampleDepth downsampleDepth_global = CreateDownsampleDepth();
const DownsampleDepth GetDownsampleDepth(){ return downsampleDepth_global; }
#endif
