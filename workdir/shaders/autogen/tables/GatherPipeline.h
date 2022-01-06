#pragma once
#include "CommandData.h"
struct GatherPipeline
{
	uint4 pip_ids[2]; // uint4
	uint commands[8]; // AppendStructuredBuffer<CommandData>
	uint4 GetPip_ids(int i) { return pip_ids[i]; }
	AppendStructuredBuffer<CommandData> GetCommands(int i) { return ResourceDescriptorHeap[commands[i]]; }
};
