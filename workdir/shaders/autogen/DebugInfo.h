#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/DebugInfo.h"
RWStructuredBuffer<DebugStruct> uav_3_0: register(u0, space3);
struct Pass_DebugInfo
{
uint uav_0;
};
ConstantBuffer<Pass_DebugInfo> pass_DebugInfo: register( b2, space3);
const DebugInfo CreateDebugInfo()
{
	DebugInfo result;
	result.uav.debug = (pass_DebugInfo.uav_0 );
	return result;
}
#ifndef NO_GLOBAL
static const DebugInfo debugInfo_global = CreateDebugInfo();
const DebugInfo GetDebugInfo(){ return debugInfo_global; }
#endif
