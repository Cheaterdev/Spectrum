#ifndef SLOT_0
	#define SLOT_0
#else
	#error Slot 0 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/FrameInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FrameInfo: register( b2, space0);
 ConstantBuffer<FrameInfo> CreateFrameInfo()
{
	return ResourceDescriptorHeap[pass_FrameInfo.offset];
}
#ifndef NO_GLOBAL
static const FrameInfo frameInfo_global = CreateFrameInfo();
const FrameInfo GetFrameInfo(){ return frameInfo_global; }
#endif
