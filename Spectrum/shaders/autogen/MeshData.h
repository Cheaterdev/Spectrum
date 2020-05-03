#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshData.h"
StructuredBuffer<mesh_vertex_input> srv_3_0: register(t0, space3);
MeshData CreateMeshData()
{
	MeshData result;
	result.srv.vb = srv_3_0;
	return result;
}
static const MeshData meshData_global = CreateMeshData();
const MeshData GetMeshData(){ return meshData_global; }
