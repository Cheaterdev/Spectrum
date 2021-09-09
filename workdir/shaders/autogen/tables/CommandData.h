#pragma once
#include "DispatchMeshArguments.h"
#include "GPUAddress.h"
struct CommandData_cb
{
	GPUAddress_cb mesh_cb;
	GPUAddress_cb material_cb;
	DispatchMeshArguments_cb draw_commands;
};
struct CommandData
{
	CommandData_cb cb;
	GPUAddress GetMesh_cb() { return CreateGPUAddress(cb.mesh_cb); }
	GPUAddress GetMaterial_cb() { return CreateGPUAddress(cb.material_cb); }
	DispatchMeshArguments GetDraw_commands() { return CreateDispatchMeshArguments(cb.draw_commands); }

};
 const CommandData CreateCommandData(CommandData_cb cb)
{
	const CommandData result = {cb
	};
	return result;
}
