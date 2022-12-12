#pragma once
#include "DispatchMeshArguments.h"
struct MeshCommandData
{
	uint material_id; // uint
	uint node_offset; // uint
	uint mesh_cb; // uint
	uint meshinstance_cb; // uint
	DispatchMeshArguments draw_commands; // DispatchMeshArguments
	uint GetMaterial_id() { return material_id; }
	uint GetNode_offset() { return node_offset; }
	uint GetMesh_cb() { return mesh_cb; }
	uint GetMeshinstance_cb() { return meshinstance_cb; }
	DispatchMeshArguments GetDraw_commands() { return draw_commands; }
};
