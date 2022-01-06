#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TextureRenderer
	{
		Render::HLSL::Texture2D<float4> texture;
		Render::HLSL::Texture2D<float4>& GetTexture() { return texture; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(texture);
		}
	};
	#pragma pack(pop)
}
