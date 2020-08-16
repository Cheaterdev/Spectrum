#pragma once
#include "BoxInfo.h"
struct GatherBoxes_uav
{
	AppendStructuredBuffer<BoxInfo> culledMeshes;
	AppendStructuredBuffer<uint> visibleMeshes;
};
struct GatherBoxes
{
	GatherBoxes_uav uav;
	AppendStructuredBuffer<BoxInfo> GetCulledMeshes() { return uav.culledMeshes; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return uav.visibleMeshes; }
};
 const GatherBoxes CreateGatherBoxes(GatherBoxes_uav uav)
{
	const GatherBoxes result = {uav
	};
	return result;
}
