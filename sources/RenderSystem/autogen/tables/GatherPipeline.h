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
		struct UAV
		{
			Render::HLSL::AppendStructuredBuffer<CommandData> commands[8];
		} &uav;
		uint4* GetPip_ids() { return cb.pip_ids; }
		Render::HLSL::AppendStructuredBuffer<CommandData>* GetCommands() { return uav.commands; }
		GatherPipeline(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
	#pragma pack(pop)
}
