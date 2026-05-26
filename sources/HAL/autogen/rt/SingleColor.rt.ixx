export module HAL:Autogen.RT.SingleColor;

import Core;
import :RT;
import :SIG;
import :HLSL;
import :Types;
import <HAL.h>;
export namespace RT
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