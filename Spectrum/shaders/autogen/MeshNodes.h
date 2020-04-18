#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshNodes.h"
StructuredBuffer<mesh_vertex_input> srv_4_0: register(t0, space4);
StructuredBuffer<node_data> srv_4_1: register(t1, space4);
MeshNodes CreateMeshNodes()
{
	MeshNodes result;
	result.srv.vb = srv_4_0;
	result.srv.nodes = srv_4_1;
	return result;
}
static const MeshNodes meshNodes_global = CreateMeshNodes();
const MeshNodes GetMeshNodes(){ return meshNodes_global; }
