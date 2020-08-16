#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
Texture2D bindless[]: register(t4, space1);
#include "tables/SceneData.h"
StructuredBuffer<node_data> srv_1_0: register(t0, space1);
StructuredBuffer<mesh_vertex_input> srv_1_1: register(t1, space1);
StructuredBuffer<MeshCommandData> srv_1_2: register(t2, space1);
StructuredBuffer<MaterialCommandData> srv_1_3: register(t3, space1);
SceneData CreateSceneData()
{
	SceneData result;
	result.srv.nodes = srv_1_0;
	result.srv.vertexes = srv_1_1;
	result.srv.meshes = srv_1_2;
	result.srv.materials = srv_1_3;
	return result;
}
static const SceneData sceneData_global = CreateSceneData();
const SceneData GetSceneData(){ return sceneData_global; }
