#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "MeshInstance.h"
#include "Meshlet.h"
#include "MeshletCullData.h"
#include "mesh_vertex_input.h"
#include "node_data.h"
struct SceneData
{
	uint nodes; // StructuredBuffer<node_data>
	uint vertexes; // StructuredBuffer<mesh_vertex_input>
	uint meshes; // StructuredBuffer<MeshCommandData>
	uint materials; // StructuredBuffer<MaterialCommandData>
	uint meshInstances; // StructuredBuffer<MeshInstance>
	uint meshlets; // StructuredBuffer<Meshlet>
	uint meshletCullData; // StructuredBuffer<MeshletCullData>
	uint indices; // StructuredBuffer<uint>
	StructuredBuffer<node_data> GetNodes() { return ResourceDescriptorHeap[nodes]; }
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return ResourceDescriptorHeap[vertexes]; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return ResourceDescriptorHeap[meshes]; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return ResourceDescriptorHeap[materials]; }
	StructuredBuffer<MeshInstance> GetMeshInstances() { return ResourceDescriptorHeap[meshInstances]; }
	StructuredBuffer<Meshlet> GetMeshlets() { return ResourceDescriptorHeap[meshlets]; }
	StructuredBuffer<MeshletCullData> GetMeshletCullData() { return ResourceDescriptorHeap[meshletCullData]; }
	StructuredBuffer<uint> GetIndices() { return ResourceDescriptorHeap[indices]; }
};
