#pragma once
#include "DebugStruct.h"
struct DebugInfo_uav
{
	RWStructuredBuffer<DebugStruct> debug;
};
struct DebugInfo
{
	DebugInfo_uav uav;
	RWStructuredBuffer<DebugStruct> GetDebug() { return uav.debug; }

};
 const DebugInfo CreateDebugInfo(DebugInfo_uav uav)
{
	const DebugInfo result = {uav
	};
	return result;
}
