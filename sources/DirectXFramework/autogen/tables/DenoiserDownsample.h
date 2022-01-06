#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserDownsample
	{
		Render::HLSL::Texture2D<float4> color;
		Render::HLSL::Texture2D<float> depth;
		Render::HLSL::Texture2D<float4>& GetColor() { return color; }
		Render::HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(depth);
		}
	};
	#pragma pack(pop)
}
