#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif

#include "layout/DefaultLayout.h"
#include "tables/FrameGraph_Debug_Common.h"

#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif

ConstantBuffer< CB > pass_FrameGraph_Debug_Common: register( b2, space4);

ConstantBuffer<FrameGraph_Debug_Common> CreateFrameGraph_Debug_Common()
{
	return ResourceDescriptorHeap[pass_FrameGraph_Debug_Common.offset];
}
			
#ifndef NO_GLOBAL
static const FrameGraph_Debug_Common frameGraph_Debug_Common_global = CreateFrameGraph_Debug_Common();
const FrameGraph_Debug_Common GetFrameGraph_Debug_Common(){ return frameGraph_Debug_Common_global; }
#endif