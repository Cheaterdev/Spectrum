#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/DebugInfo.h"
ConstantBuffer<DebugInfo> pass_DebugInfo: register( b2, space3);
const DebugInfo CreateDebugInfo()
{
	return pass_DebugInfo;
}
#ifndef NO_GLOBAL
static const DebugInfo debugInfo_global = CreateDebugInfo();
const DebugInfo GetDebugInfo(){ return debugInfo_global; }
#endif
