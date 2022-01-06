#pragma once
#include "BoxInfo.h"
struct GatherMeshesBoxes
{
	uint input_meshes; // StructuredBuffer<BoxInfo>
	uint visible_boxes; // StructuredBuffer<uint>
	uint visibleMeshes; // AppendStructuredBuffer<uint>
	uint invisibleMeshes; // AppendStructuredBuffer<uint>
	StructuredBuffer<BoxInfo> GetInput_meshes() { return ResourceDescriptorHeap[input_meshes]; }
	StructuredBuffer<uint> GetVisible_boxes() { return ResourceDescriptorHeap[visible_boxes]; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return ResourceDescriptorHeap[visibleMeshes]; }
	AppendStructuredBuffer<uint> GetInvisibleMeshes() { return ResourceDescriptorHeap[invisibleMeshes]; }
};
