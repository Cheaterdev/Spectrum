#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		struct CB
		{
			GPUAddress mesh_cb;
			GPUAddress material_cb;
			DispatchMeshArguments draw_commands;
		} &cb;
		GPUAddress& GetMesh_cb() { return cb.mesh_cb; }
		GPUAddress& GetMaterial_cb() { return cb.material_cb; }
		DispatchMeshArguments& GetDraw_commands() { return cb.draw_commands; }
		CommandData(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
