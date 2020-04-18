#pragma once
#include "mesh_vertex_input.h"
#include "node_data.h"
struct MeshNodes_srv
{
	StructuredBuffer<mesh_vertex_input> vb;
	StructuredBuffer<node_data> nodes;
};
struct MeshNodes
{
	MeshNodes_srv srv;
	StructuredBuffer<mesh_vertex_input> GetVb() { return srv.vb; }
	StructuredBuffer<node_data> GetNodes() { return srv.nodes; }
};
 const MeshNodes CreateMeshNodes(MeshNodes_srv srv)
{
	const MeshNodes result = {srv
	};
	return result;
}
