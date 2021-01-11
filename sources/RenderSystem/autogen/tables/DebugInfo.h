#pragma once
#include "DebugStruct.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugInfo
	{
		struct UAV
		{
			DX12::HLSL::RWStructuredBuffer<DebugStruct> debug;
		} &uav;
		DX12::HLSL::RWStructuredBuffer<DebugStruct>& GetDebug() { return uav.debug; }
		DebugInfo(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
