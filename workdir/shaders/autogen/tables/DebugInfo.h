#pragma once
#include "sig_hlsl.hlsl"
#include "DebugStruct.h"
struct DebugInfo
{
	uint debug; // RWStructuredBuffer<DebugStruct>
	RWStructuredBuffer<DebugStruct> GetDebug() { return ResourceDescriptorHeap[debug]; }
	
		void Log(uint id, uint4 v)
		{
			DebugStruct debug;

            debug.v = v;

            uav.debug[id] = debug;
		}
		
	
};
