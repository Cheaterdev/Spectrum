#pragma once
#include "Meshlet.h"
#include "MeshletCullData.h"
#include "mesh_vertex_input.h"
struct MeshInstanceInfo
{
	uint vertexes; // StructuredBuffer<mesh_vertex_input>
	uint indices; // StructuredBuffer<uint>
	uint meshlets; // StructuredBuffer<Meshlet>
	uint meshletCullData; // StructuredBuffer<MeshletCullData>
	uint unique_indices; // StructuredBuffer<uint>
	uint primitive_indices; // StructuredBuffer<uint>
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return ResourceDescriptorHeap[vertexes]; }
	StructuredBuffer<uint> GetIndices() { return ResourceDescriptorHeap[indices]; }
	StructuredBuffer<Meshlet> GetMeshlets() { return ResourceDescriptorHeap[meshlets]; }
	StructuredBuffer<MeshletCullData> GetMeshletCullData() { return ResourceDescriptorHeap[meshletCullData]; }
	StructuredBuffer<uint> GetUnique_indices() { return ResourceDescriptorHeap[unique_indices]; }
	StructuredBuffer<uint> GetPrimitive_indices() { return ResourceDescriptorHeap[primitive_indices]; }
};
