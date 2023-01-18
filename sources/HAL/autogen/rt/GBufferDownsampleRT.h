#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _GBufferDownsampleRT
	{
		HLSL::RenderTarget<float> depth;
		HLSL::RenderTarget<float4> color;
		HLSL::RenderTarget<float>& GetDepth() { return depth; }
		HLSL::RenderTarget<float4>& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
			compiler.compile(color);
		}
	};
	#pragma pack(pop)
	struct GBufferDownsampleRT:public RTHolder<_GBufferDownsampleRT>
	{
		GBufferDownsampleRT() = default;
	};
}
