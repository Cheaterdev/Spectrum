#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		struct CB
		{
			GPUAddress material_cb;
			GPUAddress mesh_cb;
			DrawIndexedArguments draw_commands;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		GPUAddress& GetMaterial_cb() { return cb.material_cb; }
		GPUAddress& GetMesh_cb() { return cb.mesh_cb; }
		DrawIndexedArguments& GetDraw_commands() { return cb.draw_commands; }
		CommandData(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
