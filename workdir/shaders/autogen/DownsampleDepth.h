#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DownsampleDepth.h"
Texture2D<float> srv_4_0: register(t0, space4);
RWTexture2D<float> uav_4_0: register(u0, space4);
struct Pass_DownsampleDepth
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_DownsampleDepth> pass_DownsampleDepth: register( b2, space4);
const DownsampleDepth CreateDownsampleDepth()
{
	DownsampleDepth result;
	result.srv.srcTex = (pass_DownsampleDepth.srv_0 );
	result.uav.targetTex = (pass_DownsampleDepth.uav_0 );
	return result;
}
#ifndef NO_GLOBAL
static const DownsampleDepth downsampleDepth_global = CreateDownsampleDepth();
const DownsampleDepth GetDownsampleDepth(){ return downsampleDepth_global; }
#endif
