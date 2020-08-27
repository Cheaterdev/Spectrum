#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "MeshInstance.h"
#include "mesh_vertex_input.h"
#include "node_data.h"
struct SceneData_srv
{
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<mesh_vertex_input> vertexes;
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;
	StructuredBuffer<MeshInstance> meshInstances;
};
struct SceneData
{
	SceneData_srv srv;
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return srv.vertexes; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return srv.meshes; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return srv.materials; }
	StructuredBuffer<MeshInstance> GetMeshInstances() { return srv.meshInstances; }
	Texture2D GetMaterial_textures(int i) { return bindless[i]; }

};
 const SceneData CreateSceneData(SceneData_srv srv)
{
	const SceneData result = {srv
	};
	return result;
}
