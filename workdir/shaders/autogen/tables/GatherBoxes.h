#pragma once
#include "BoxInfo.h"
struct GatherBoxes_uav
{
	uint culledMeshes; // AppendStructuredBuffer<BoxInfo>
	uint visibleMeshes; // AppendStructuredBuffer<uint>
};
struct GatherBoxes
{
	GatherBoxes_uav uav;
	AppendStructuredBuffer<BoxInfo> GetCulledMeshes() { return ResourceDescriptorHeap[uav.culledMeshes]; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return ResourceDescriptorHeap[uav.visibleMeshes]; }

};
 const GatherBoxes CreateGatherBoxes(GatherBoxes_uav uav)
{
	const GatherBoxes result = {uav
	};
	return result;
}
