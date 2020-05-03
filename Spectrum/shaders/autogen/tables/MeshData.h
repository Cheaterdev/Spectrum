#pragma once
#include "mesh_vertex_input.h"
struct MeshData_srv
{
	StructuredBuffer<mesh_vertex_input> vb;
};
struct MeshData
{
	MeshData_srv srv;
	StructuredBuffer<mesh_vertex_input> GetVb() { return srv.vb; }
};
 const MeshData CreateMeshData(MeshData_srv srv)
{
	const MeshData result = {srv
	};
	return result;
}
