#ifndef SLOT_0
	#define SLOT_0
#else
	#error Slot 0 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/FrameInfo.h"
ConstantBuffer<FrameInfo_cb> cb_0_0:register(b0,space0);
Texture2D srv_0_0: register(t0, space0);
FrameInfo CreateFrameInfo()
{
	FrameInfo result;
	result.cb = cb_0_0;
	result.srv.bestFitNormals = srv_0_0;
	result.camera = CreateCamera(result.cb.camera);
	result.prevCamera = CreateCamera(result.cb.prevCamera);
	return result;
}
static const FrameInfo frameInfo_global = CreateFrameInfo();
const FrameInfo GetFrameInfo(){ return frameInfo_global; }
