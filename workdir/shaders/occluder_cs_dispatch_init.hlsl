#include "autogen/InitDispatch.h"
#include "autogen/GatherPipelineGlobal.h"

static const InitDispatch initDispatch = GetInitDispatch();
static const RWStructuredBuffer<DispatchArguments> result = initDispatch.GetDispatch_data();
static const GatherPipelineGlobal pip = GetGatherPipelineGlobal();

[numthreads(1, 1, 1)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex
)
{
	uint max_count = pip.GetMeshes_count()[0];

	DispatchArguments dinfo;
	dinfo.counts =  uint3(((max_count + 64 - 1) / (64)), 1, 1);

	result[0] = dinfo;
}


