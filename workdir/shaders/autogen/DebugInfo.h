#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/DebugInfo.h"
RWStructuredBuffer<DebugStruct> uav_2_0: register(u0, space2);
DebugInfo CreateDebugInfo()
{
	DebugInfo result;
	result.uav.debug = uav_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const DebugInfo debugInfo_global = CreateDebugInfo();
const DebugInfo GetDebugInfo(){ return debugInfo_global; }
#endif
