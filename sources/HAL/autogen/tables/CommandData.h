#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		GPUAddress mesh_cb;
		GPUAddress material_cb;
		DispatchMeshArguments draw_commands;
		GPUAddress& GetMesh_cb() { return mesh_cb; }
		GPUAddress& GetMaterial_cb() { return material_cb; }
		DispatchMeshArguments& GetDraw_commands() { return draw_commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(mesh_cb);
			compiler.compile(material_cb);
			compiler.compile(draw_commands);
		}
	};
	#pragma pack(pop)
}
