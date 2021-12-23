#pragma once
#include "CommandData.h"
struct GatherPipeline_cb
{
	uint4 pip_ids[2]; // uint4
};
struct GatherPipeline_uav
{
	uint commands[8]; // AppendStructuredBuffer<CommandData>
};
struct GatherPipeline
{
	GatherPipeline_cb cb;
	GatherPipeline_uav uav;
	AppendStructuredBuffer<CommandData> GetCommands(int i) { return ResourceDescriptorHeap[uav.commands[i]]; }
	uint4 GetPip_ids(int i) { return cb.pip_ids[i]; }

};
 const GatherPipeline CreateGatherPipeline(GatherPipeline_cb cb,GatherPipeline_uav uav)
{
	const GatherPipeline result = {cb,uav
	};
	return result;
}
