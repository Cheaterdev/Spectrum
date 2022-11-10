#pragma once
#include "DispatchMeshArguments.h"
#include "GPUAddress.h"
struct MeshCommandData
{
	uint material_id; // uint
	uint node_offset; // uint
	GPUAddress mesh_cb; // GPUAddress
	DispatchMeshArguments draw_commands; // DispatchMeshArguments
	uint GetMaterial_id() { return material_id; }
	uint GetNode_offset() { return node_offset; }
	GPUAddress GetMesh_cb() { return mesh_cb; }
	DispatchMeshArguments GetDraw_commands() { return draw_commands; }
};
