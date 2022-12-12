#pragma once
#include "mesh_vertex_input.h"
struct RaytraceInstanceInfo
{
	uint vertexes; // StructuredBuffer<mesh_vertex_input>
	uint indices; // StructuredBuffer<uint>
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return ResourceDescriptorHeap[vertexes]; }
	StructuredBuffer<uint> GetIndices() { return ResourceDescriptorHeap[indices]; }
};
