#pragma once
#include "DispatchMeshArguments.h"
#include "MeshInfo.h"
#include "MeshInstanceInfo.h"
struct CommandData
{
	uint mesh_cb; // MeshInfo
	uint meshinstance_cb; // MeshInstanceInfo
	uint material_cb; // MaterialInfo
	DispatchMeshArguments draw_commands; // DispatchMeshArguments
	uint GetMesh_cb() { return mesh_cb; }
	uint GetMeshinstance_cb() { return meshinstance_cb; }
	uint GetMaterial_cb() { return material_cb; }
	DispatchMeshArguments GetDraw_commands() { return draw_commands; }
};
