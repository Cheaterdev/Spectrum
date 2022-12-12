#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshCommandData
	{
		uint material_id;
		uint node_offset;
		uint mesh_cb;
		uint meshinstance_cb;
		DispatchMeshArguments draw_commands;
		uint& GetMaterial_id() { return material_id; }
		uint& GetNode_offset() { return node_offset; }
		uint& GetMesh_cb() { return mesh_cb; }
		uint& GetMeshinstance_cb() { return meshinstance_cb; }
		DispatchMeshArguments& GetDraw_commands() { return draw_commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(material_id);
			compiler.compile(node_offset);
			compiler.compile(mesh_cb);
			compiler.compile(meshinstance_cb);
			compiler.compile(draw_commands);
		}
		using Compiled = MeshCommandData;
		};
	#pragma pack(pop)
}
