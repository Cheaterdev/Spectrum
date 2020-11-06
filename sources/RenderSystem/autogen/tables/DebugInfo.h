#pragma once
#include "DebugStruct.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugInfo
	{
		using CB = Empty;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle debug;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetDebug() { return uav.debug; }
		DebugInfo(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
