#pragma once
#include "DispatchMeshArguments.h"
#include "GPUAddress.h"
struct CommandData
{
	GPUAddress mesh_cb; // GPUAddress
	GPUAddress material_cb; // GPUAddress
	DispatchMeshArguments draw_commands; // DispatchMeshArguments
	GPUAddress GetMesh_cb() { return mesh_cb; }
	GPUAddress GetMaterial_cb() { return material_cb; }
	DispatchMeshArguments GetDraw_commands() { return draw_commands; }
};
