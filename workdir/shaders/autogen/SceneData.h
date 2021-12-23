#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
Texture2D<float4> bindless[]: register(t8, space1);
#include "tables/SceneData.h"
StructuredBuffer<node_data> srv_1_0: register(t0, space1);
StructuredBuffer<mesh_vertex_input> srv_1_1: register(t1, space1);
StructuredBuffer<MeshCommandData> srv_1_2: register(t2, space1);
StructuredBuffer<MaterialCommandData> srv_1_3: register(t3, space1);
StructuredBuffer<MeshInstance> srv_1_4: register(t4, space1);
StructuredBuffer<Meshlet> srv_1_5: register(t5, space1);
StructuredBuffer<MeshletCullData> srv_1_6: register(t6, space1);
StructuredBuffer<uint> srv_1_7: register(t7, space1);
struct Pass_SceneData
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint srv_5;
uint srv_6;
uint srv_7;
};
ConstantBuffer<Pass_SceneData> pass_SceneData: register( b2, space1);
const SceneData CreateSceneData()
{
	SceneData result;
	result.srv.nodes = (pass_SceneData.srv_0 );
	result.srv.vertexes = (pass_SceneData.srv_1 );
	result.srv.meshes = (pass_SceneData.srv_2 );
	result.srv.materials = (pass_SceneData.srv_3 );
	result.srv.meshInstances = (pass_SceneData.srv_4 );
	result.srv.meshlets = (pass_SceneData.srv_5 );
	result.srv.meshletCullData = (pass_SceneData.srv_6 );
	result.srv.indices = (pass_SceneData.srv_7 );
	return result;
}
#ifndef NO_GLOBAL
static const SceneData sceneData_global = CreateSceneData();
const SceneData GetSceneData(){ return sceneData_global; }
#endif
