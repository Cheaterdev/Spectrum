#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
Texture2D bindless[]: register(t3, space1);
#include "tables/SceneData.h"
Buffer<uint> srv_1_0: register(t0, space1);
StructuredBuffer<node_data> srv_1_1: register(t1, space1);
StructuredBuffer<mesh_vertex_input> srv_1_2: register(t2, space1);
SceneData CreateSceneData()
{
	SceneData result;
	result.srv.commands = srv_1_0;
	result.srv.nodes = srv_1_1;
	result.srv.vertexes = srv_1_2;
	return result;
}
static const SceneData sceneData_global = CreateSceneData();
const SceneData GetSceneData(){ return sceneData_global; }
