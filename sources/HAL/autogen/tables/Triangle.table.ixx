export module HAL:Autogen.Tables.Triangle;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Autogen.Tables.mesh_vertex_input;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Triangle
	{
		static constexpr SlotID ID = SlotID::Triangle;
		float lod;
		mesh_vertex_input v;
		float& GetLod() { return lod; }
		mesh_vertex_input& GetV() { return v; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(lod);
			compiler.compile(v);
		}
		using Compiled = Triangle;
		};
		#pragma pack(pop)
	}