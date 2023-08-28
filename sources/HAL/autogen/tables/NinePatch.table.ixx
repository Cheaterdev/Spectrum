export module HAL:Autogen.Tables.NinePatch;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.vertex_input;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct NinePatch
	{
		static constexpr SlotID ID = SlotID::NinePatch;
		HLSL::StructuredBuffer<vertex_input> vb;
		std::vector<HLSL::Texture2D<float4>> textures;
		HLSL::StructuredBuffer<vertex_input>& GetVb() { return vb; }
		std::vector<HLSL::Texture2D<float4>>& GetTextures() { return textures; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vb);
			compiler.compile(textures);
		}
		struct Compiled
		{
			uint vb; // StructuredBuffer<vertex_input>
			uint textures; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
