#pragma once
#include "DispatchMeshArguments.h"
#include "GPUAddress.h"
struct MeshCommandData_cb
{
	uint material_id; // uint
	uint node_offset; // uint
	GPUAddress_cb mesh_cb;
	DispatchMeshArguments_cb draw_commands;
};
struct MeshCommandData
{
	MeshCommandData_cb cb;
	uint GetMaterial_id() { return cb.material_id; }
	uint GetNode_offset() { return cb.node_offset; }
	GPUAddress GetMesh_cb() { return CreateGPUAddress(cb.mesh_cb); }
	DispatchMeshArguments GetDraw_commands() { return CreateDispatchMeshArguments(cb.draw_commands); }

};
 const MeshCommandData CreateMeshCommandData(MeshCommandData_cb cb)
{
	const MeshCommandData result = {cb
	};
	return result;
}
