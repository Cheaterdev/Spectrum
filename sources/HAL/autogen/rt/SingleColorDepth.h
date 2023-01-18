#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _SingleColorDepth
	{
		HLSL::RenderTarget<float4> color;
		HLSL::DepthStencil<float> depth;
		HLSL::RenderTarget<float4>& GetColor() { return color; }
		HLSL::DepthStencil<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(depth);
		}
	};
	#pragma pack(pop)
	struct SingleColorDepth:public RTHolder<_SingleColorDepth>
	{
		SingleColorDepth() = default;
	};
}
