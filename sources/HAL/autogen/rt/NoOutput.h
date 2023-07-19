#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _NoOutput
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::RT;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
		}
	};
	#pragma pack(pop)
	struct NoOutput:public RTHolder<_NoOutput>
	{
		NoOutput() = default;
	};
}
