#ifndef SLOT_0
	#define SLOT_0
#else
	#error Slot 0 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/FrameInfo.h"
ConstantBuffer<FrameInfo> pass_FrameInfo: register( b2, space0);
const FrameInfo CreateFrameInfo()
{
	return pass_FrameInfo;
}
#ifndef NO_GLOBAL
static const FrameInfo frameInfo_global = CreateFrameInfo();
const FrameInfo GetFrameInfo(){ return frameInfo_global; }
#endif
