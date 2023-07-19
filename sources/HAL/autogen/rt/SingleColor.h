#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _SingleColor
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::RT;
		HLSL::RenderTarget<float4> color;
		HLSL::RenderTarget<float4>& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
		}
	};
	#pragma pack(pop)
	struct SingleColor:public RTHolder<_SingleColor>
	{
		SingleColor() = default;
	};
}
