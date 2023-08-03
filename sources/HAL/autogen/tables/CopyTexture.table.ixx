export module HAL:Autogen.Tables.CopyTexture;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct CopyTexture
	{
		static constexpr SlotID ID = SlotID::CopyTexture;
		HLSL::Texture2D<float4> srcTex;
		HLSL::Texture2D<float4>& GetSrcTex() { return srcTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(srcTex);
		}
		struct Compiled
		{
			uint srcTex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
