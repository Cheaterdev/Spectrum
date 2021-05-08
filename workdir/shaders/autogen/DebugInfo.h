#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/DebugInfo.h"
RWStructuredBuffer<DebugStruct> uav_2_0: register(u0, space2);
struct Pass_DebugInfo
{
uint uav_0;
};
ConstantBuffer<Pass_DebugInfo> pass_DebugInfo: register( b2, space2);
const DebugInfo CreateDebugInfo()
{
	DebugInfo result;
	Pass_DebugInfo pass;
	result.uav.debug = uav_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const DebugInfo debugInfo_global = CreateDebugInfo();
const DebugInfo GetDebugInfo(){ return debugInfo_global; }
#endif
