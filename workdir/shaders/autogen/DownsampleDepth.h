#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DownsampleDepth.h"
Texture2D<float> srv_3_0: register(t0, space3);
RWTexture2D<float> uav_3_0: register(u0, space3);
struct Pass_DownsampleDepth
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_DownsampleDepth> pass_DownsampleDepth: register( b2, space3);
const DownsampleDepth CreateDownsampleDepth()
{
	DownsampleDepth result;
	Pass_DownsampleDepth pass;
	result.srv.srcTex = srv_3_0;
	result.uav.targetTex = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const DownsampleDepth downsampleDepth_global = CreateDownsampleDepth();
const DownsampleDepth GetDownsampleDepth(){ return downsampleDepth_global; }
#endif
