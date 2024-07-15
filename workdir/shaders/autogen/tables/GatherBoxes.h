#pragma once
#include "sig_hlsl.hlsl"
#include "BoxInfo.h"
struct GatherBoxes
{
	uint culledMeshes; // AppendStructuredBuffer<BoxInfo>
	uint visibleMeshes; // AppendStructuredBuffer<uint>
	AppendStructuredBuffer<BoxInfo> GetCulledMeshes() { return ResourceDescriptorHeap[culledMeshes]; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return ResourceDescriptorHeap[visibleMeshes]; }
};