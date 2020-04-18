#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshData.h"
StructuredBuffer<mesh_vertex_input> srv_2_0: register(t0, space2);
StructuredBuffer<node_data> srv_2_1: register(t1, space2);
MeshData CreateMeshData()
{
	MeshData result;
	result.srv.vb = srv_2_0;
	result.srv.nodes = srv_2_1;
	return result;
}
static const MeshData meshData_global = CreateMeshData();
const MeshData GetMeshData(){ return meshData_global; }
