#pragma once
#include "DispatchMeshArguments.h"
struct CommandData
{
	uint mesh_cb; // uint
	uint material_cb; // uint
	DispatchMeshArguments draw_commands; // DispatchMeshArguments
	uint GetMesh_cb() { return mesh_cb; }
	uint GetMaterial_cb() { return material_cb; }
	DispatchMeshArguments GetDraw_commands() { return draw_commands; }
};
