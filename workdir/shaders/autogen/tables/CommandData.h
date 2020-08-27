#pragma once
#include "DrawIndexedArguments.h"
#include "GPUAddress.h"
struct CommandData_cb
{
	GPUAddress_cb mesh_cb;
	GPUAddress_cb material_cb;
	DrawIndexedArguments_cb draw_commands;
};
struct CommandData
{
	CommandData_cb cb;
	GPUAddress GetMesh_cb() { return CreateGPUAddress(cb.mesh_cb); }
	GPUAddress GetMaterial_cb() { return CreateGPUAddress(cb.material_cb); }
	DrawIndexedArguments GetDraw_commands() { return CreateDrawIndexedArguments(cb.draw_commands); }

};
 const CommandData CreateCommandData(CommandData_cb cb)
{
	const CommandData result = {cb
	};
	return result;
}
