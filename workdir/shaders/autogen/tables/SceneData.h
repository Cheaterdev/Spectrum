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
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<mesh_vertex_input> vertexes;
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;
	StructuredBuffer<MeshInstance> meshInstances;
	StructuredBuffer<Meshlet> meshlets;
	StructuredBuffer<MeshletCullData> meshletCullData;
	StructuredBuffer<uint> indices;
};
struct SceneData
{
	SceneData_srv srv;
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return srv.vertexes; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return srv.meshes; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return srv.materials; }
	StructuredBuffer<MeshInstance> GetMeshInstances() { return srv.meshInstances; }
	StructuredBuffer<Meshlet> GetMeshlets() { return srv.meshlets; }
	StructuredBuffer<MeshletCullData> GetMeshletCullData() { return srv.meshletCullData; }
	StructuredBuffer<uint> GetIndices() { return srv.indices; }
	Texture2D<float4> GetMaterial_textures(int i) { return bindless[i]; }

};
 const SceneData CreateSceneData(SceneData_srv srv)
{
	const SceneData result = {srv
	};
	return result;
}
