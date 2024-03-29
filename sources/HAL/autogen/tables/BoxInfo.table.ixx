export module HAL:Autogen.Tables.BoxInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct BoxInfo
	{
		static constexpr SlotID ID = SlotID::BoxInfo;
		uint node_offset;
		uint mesh_id;
		uint& GetNode_offset() { return node_offset; }
		uint& GetMesh_id() { return mesh_id; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(node_offset);
			compiler.compile(mesh_id);
		}
		using Compiled = BoxInfo;
		};
		#pragma pack(pop)
	}
