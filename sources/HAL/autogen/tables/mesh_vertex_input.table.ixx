export module HAL:Autogen.Tables.mesh_vertex_input;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct mesh_vertex_input
	{
		static constexpr SlotID ID = SlotID::mesh_vertex_input;
		float3 pos;
		float3 normal;
		float2 tc;
		float4 tangent;
		float3& GetPos() { return pos; }
		float3& GetNormal() { return normal; }
		float2& GetTc() { return tc; }
		float4& GetTangent() { return tangent; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pos);
			compiler.compile(normal);
			compiler.compile(tc);
			compiler.compile(tangent);
		}
		using Compiled = mesh_vertex_input;
		private:
	SERIALIZE()
		{
		     ar& NVP(pos);
		     ar& NVP(normal);
		     ar& NVP(tc);
		     ar& NVP(tangent);
		}
		};
		#pragma pack(pop)
	}
