#pragma once
#include "DrawIndexedArguments.h"
#include "GPUAddress.h"
struct MeshCommandData_cb
{
	uint material_id;
	uint node_offset;
	GPUAddress_cb mesh_cb;
	DrawIndexedArguments_cb draw_commands;
};
struct MeshCommandData
{
	MeshCommandData_cb cb;
	uint GetMaterial_id() { return cb.material_id; }
	uint GetNode_offset() { return cb.node_offset; }
	GPUAddress GetMesh_cb() { return CreateGPUAddress(cb.mesh_cb); }
	DrawIndexedArguments GetDraw_commands() { return CreateDrawIndexedArguments(cb.draw_commands); }

};
 const MeshCommandData CreateMeshCommandData(MeshCommandData_cb cb)
{
	const MeshCommandData result = {cb
	};
	return result;
}
