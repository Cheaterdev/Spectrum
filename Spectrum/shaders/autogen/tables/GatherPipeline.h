#pragma once
#include "CommandData.h"
struct GatherPipeline_cb
{
	uint4 pip_ids[2];
};
struct GatherPipeline_uav
{
	AppendStructuredBuffer<CommandData> commands[8];
};
struct GatherPipeline
{
	GatherPipeline_cb cb;
	GatherPipeline_uav uav;
	//uint GetPip_ids(int i) { return cb.pip_ids[i]; }
	AppendStructuredBuffer<CommandData> GetCommands(int i) { return uav.commands[i]; }
};
 const GatherPipeline CreateGatherPipeline(GatherPipeline_cb cb,GatherPipeline_uav uav)
{
	const GatherPipeline result = {cb,uav
	};
	return result;
}
