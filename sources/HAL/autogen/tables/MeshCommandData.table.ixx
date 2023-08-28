export module HAL:Autogen.Tables.MeshCommandData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.MeshInfo;
import :Autogen.Tables.MeshInstanceInfo;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MeshCommandData
	{
		static constexpr SlotID ID = SlotID::MeshCommandData;
		uint material_id;
		uint node_offset;
		Pointer<MeshInfo> mesh_cb;
		Pointer<MeshInstanceInfo> meshinstance_cb;
		DispatchMeshArguments draw_commands;
		uint& GetMaterial_id() { return material_id; }
		uint& GetNode_offset() { return node_offset; }
		Pointer<MeshInfo>& GetMesh_cb() { return mesh_cb; }
		Pointer<MeshInstanceInfo>& GetMeshinstance_cb() { return meshinstance_cb; }
		DispatchMeshArguments& GetDraw_commands() { return draw_commands; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
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
