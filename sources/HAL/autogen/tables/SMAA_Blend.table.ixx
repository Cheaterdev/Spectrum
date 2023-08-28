export module HAL:Autogen.Tables.SMAA_Blend;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		static constexpr SlotID ID = SlotID::SMAA_Blend;
		HLSL::Texture2D<float4> blendTex;
		HLSL::Texture2D<float4>& GetBlendTex() { return blendTex; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(blendTex);
		}
		struct Compiled
		{
			uint blendTex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
