#pragma once
#include "BoxInfo.h"
struct DrawBoxes
{
	uint vertices; // StructuredBuffer<float4>
	uint input_meshes; // StructuredBuffer<BoxInfo>
	uint visible_meshes; // RWStructuredBuffer<uint>
	StructuredBuffer<float4> GetVertices() { return ResourceDescriptorHeap[vertices]; }
	StructuredBuffer<BoxInfo> GetInput_meshes() { return ResourceDescriptorHeap[input_meshes]; }
	RWStructuredBuffer<uint> GetVisible_meshes() { return ResourceDescriptorHeap[visible_meshes]; }
};
