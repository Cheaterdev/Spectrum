#pragma once
#include "mesh_vertex_input.h"
#include "node_data.h"
struct MeshData_srv
{
	StructuredBuffer<mesh_vertex_input> vb;
	StructuredBuffer<node_data> nodes;
};
struct MeshData
{
	MeshData_srv srv;
	StructuredBuffer<mesh_vertex_input> GetVb() { return srv.vb; }
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
};
 const MeshData CreateMeshData(MeshData_srv srv)
{
	const MeshData result = {srv
	};
	return result;
}
