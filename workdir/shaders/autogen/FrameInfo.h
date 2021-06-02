#ifndef SLOT_0
	#define SLOT_0
#else
	#error Slot 0 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/FrameInfo.h"
ConstantBuffer<FrameInfo_cb> cb_0_0:register(b0,space0);
Texture2D<float4> srv_0_0: register(t0, space0);
Texture3D<float4> srv_0_1: register(t1, space0);
TextureCube<float4> srv_0_2: register(t2, space0);
struct Pass_FrameInfo
{
uint srv_0;
uint srv_1;
uint srv_2;
};
ConstantBuffer<Pass_FrameInfo> pass_FrameInfo: register( b2, space0);
const FrameInfo CreateFrameInfo()
{
	FrameInfo result;
	result.cb = cb_0_0;
	result.srv.bestFitNormals = srv_0_0;
	result.srv.brdf = srv_0_1;
	result.srv.sky = srv_0_2;
	return result;
}
#ifndef NO_GLOBAL
static const FrameInfo frameInfo_global = CreateFrameInfo();
const FrameInfo GetFrameInfo(){ return frameInfo_global; }
#endif
