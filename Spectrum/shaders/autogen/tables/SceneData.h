#pragma once
#include "node_data.h"
struct SceneData_srv
{
	StructuredBuffer<node_data> nodes;
};
struct SceneData
{
	SceneData_srv srv;
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
	Texture2D GetMaterial_textures(int i) { return bindless[i]; }
};
 const SceneData CreateSceneData(SceneData_srv srv)
{
	const SceneData result = {srv
	};
	return result;
}
