#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "MeshInstance.h"
#include "Meshlet.h"
#include "MeshletCullData.h"
#include "mesh_vertex_input.h"
#include "node_data.h"
struct SceneData_srv
{
	uint nodes; // StructuredBuffer<node_data>
	uint vertexes; // StructuredBuffer<mesh_vertex_input>
	uint meshes; // StructuredBuffer<MeshCommandData>
	uint materials; // StructuredBuffer<MaterialCommandData>
	uint meshInstances; // StructuredBuffer<MeshInstance>
	uint meshlets; // StructuredBuffer<Meshlet>
	uint meshletCullData; // StructuredBuffer<MeshletCullData>
	uint indices; // StructuredBuffer<uint>
};
struct SceneData
{
	SceneData_srv srv;
	StructuredBuffer<node_data> GetNodes() { return ResourceDescriptorHeap[srv.nodes]; }
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return ResourceDescriptorHeap[srv.vertexes]; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return ResourceDescriptorHeap[srv.meshes]; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return ResourceDescriptorHeap[srv.materials]; }
	StructuredBuffer<MeshInstance> GetMeshInstances() { return ResourceDescriptorHeap[srv.meshInstances]; }
	StructuredBuffer<Meshlet> GetMeshlets() { return ResourceDescriptorHeap[srv.meshlets]; }
	StructuredBuffer<MeshletCullData> GetMeshletCullData() { return ResourceDescriptorHeap[srv.meshletCullData]; }
	StructuredBuffer<uint> GetIndices() { return ResourceDescriptorHeap[srv.indices]; }
	Texture2D<float4> GetMaterial_textures(int i) { return bindless[i]; }

};
 const SceneData CreateSceneData(SceneData_srv srv)
{
	const SceneData result = {srv
	};
	return result;
}
