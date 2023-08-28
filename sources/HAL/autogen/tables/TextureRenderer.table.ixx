export module HAL:Autogen.Tables.TextureRenderer;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct TextureRenderer
	{
		static constexpr SlotID ID = SlotID::TextureRenderer;
		HLSL::Texture2D<float4> texture;
		HLSL::Texture2D<float4>& GetTexture() { return texture; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(texture);
		}
		struct Compiled
		{
			uint texture; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
