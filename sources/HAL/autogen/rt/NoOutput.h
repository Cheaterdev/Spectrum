#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _NoOutput
	{
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
