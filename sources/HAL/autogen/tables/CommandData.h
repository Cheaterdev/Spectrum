#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		uint mesh_cb;
		uint material_cb;
		DispatchMeshArguments draw_commands;
		uint& GetMesh_cb() { return mesh_cb; }
		uint& GetMaterial_cb() { return material_cb; }
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
