#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/DebugInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DebugInfo: register( b2, space3);
 ConstantBuffer<DebugInfo> CreateDebugInfo()
{
	return ResourceDescriptorHeap[pass_DebugInfo.offset];
}
#ifndef NO_GLOBAL
static const DebugInfo debugInfo_global = CreateDebugInfo();
const DebugInfo GetDebugInfo(){ return debugInfo_global; }
#endif
