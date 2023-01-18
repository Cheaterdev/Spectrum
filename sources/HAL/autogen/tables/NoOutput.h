#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct NoOutput
	{
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
		}
		using Compiled = NoOutput;
		};
		#pragma pack(pop)
	}
