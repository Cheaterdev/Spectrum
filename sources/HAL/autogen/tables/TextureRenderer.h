#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TextureRenderer
	{
		static constexpr SlotID ID = SlotID::TextureRenderer;
		HLSL::Texture2D<float4> texture;
		HLSL::Texture2D<float4>& GetTexture() { return texture; }
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
