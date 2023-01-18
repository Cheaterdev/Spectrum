#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsampleRT
	{
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		HLSL::Texture2D<float4>& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
			compiler.compile(color);
		}
		struct Compiled
		{
			uint depth; // RenderTarget<float>
			uint color; // RenderTarget<float4>
		};
	};
	#pragma pack(pop)
}
