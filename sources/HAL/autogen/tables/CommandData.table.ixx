export module HAL:Autogen.Tables.CommandData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		static constexpr SlotID ID = SlotID::CommandData;
		uint mesh_cb;
		uint meshinstance_cb;
		uint material_cb;
		DispatchMeshArguments draw_commands;
		uint& GetMesh_cb() { return mesh_cb; }
		uint& GetMeshinstance_cb() { return meshinstance_cb; }
		uint& GetMaterial_cb() { return material_cb; }
		DispatchMeshArguments& GetDraw_commands() { return draw_commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(mesh_cb);
			compiler.compile(meshinstance_cb);
			compiler.compile(material_cb);
			compiler.compile(draw_commands);
		}
		using Compiled = CommandData;
		};
		#pragma pack(pop)
	}
