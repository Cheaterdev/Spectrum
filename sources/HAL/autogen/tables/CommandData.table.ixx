export module HAL:Autogen.Tables.CommandData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Autogen.Tables.MaterialInfo;
import :Autogen.Tables.MeshInfo;
import :Autogen.Tables.MeshInstanceInfo;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct CommandData
	{
		static constexpr SlotID ID = SlotID::CommandData;
		Pointer<MeshInfo> mesh_cb;
		Pointer<MeshInstanceInfo> meshinstance_cb;
		Pointer<MaterialInfo> material_cb;
		DispatchMeshArguments draw_commands;
		Pointer<MeshInfo>& GetMesh_cb() { return mesh_cb; }
		Pointer<MeshInstanceInfo>& GetMeshinstance_cb() { return meshinstance_cb; }
		Pointer<MaterialInfo>& GetMaterial_cb() { return material_cb; }
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
