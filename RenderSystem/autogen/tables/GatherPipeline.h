#pragma once
#include "CommandData.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipeline
	{
		struct CB
		{
			uint4 pip_ids[2];
		} &cb;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle commands[8];
		} &uav;
		using SMP = Empty;
		uint4* GetPip_ids() { return cb.pip_ids; }
		Render::Handle* GetCommands() { return uav.commands; }
		GatherPipeline(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
	#pragma pack(pop)
}
