export module HAL:Autogen.RT.DepthOnly;

import Core;
import :RT;
import :SIG;
import :HLSL;
import :Types;
import <HAL.h>;
export namespace RT
{
	#pragma pack(push, 1)
	struct _DepthOnly
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::RT;
		HLSL::DepthStencil<float> depth;
		HLSL::DepthStencil<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
		}
	};
	#pragma pack(pop)

	struct DepthOnly:public RTHolder<_DepthOnly>
	{
		DepthOnly() = default;
	};
}