#pragma once
#include "mesh_vertex_input.h"
#include "node_data.h"
struct SceneData_srv
{
	Buffer<uint> commands;
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<mesh_vertex_input> vertexes;
};
struct SceneData
{
	SceneData_srv srv;
	Buffer<uint> GetCommands() { return srv.commands; }
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
	StructuredBuffer<mesh_vertex_input> GetVertexes() { return srv.vertexes; }
	Texture2D GetMaterial_textures(int i) { return bindless[i]; }
};
 const SceneData CreateSceneData(SceneData_srv srv)
{
	const SceneData result = {srv
	};
	return result;
}
