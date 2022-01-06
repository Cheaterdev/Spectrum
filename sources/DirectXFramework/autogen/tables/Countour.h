#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Countour
	{
		float4 color;
		Render::HLSL::Texture2D<float4> tex;
		float4& GetColor() { return color; }
		Render::HLSL::Texture2D<float4>& GetTex() { return tex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(tex);
		}
	};
	#pragma pack(pop)
}
