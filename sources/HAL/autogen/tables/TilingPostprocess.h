#pragma once
#include "TilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingPostprocess
	{
		TilingParams tiling;
		TilingParams& GetTiling() { return tiling; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tiling);
		}
		using Compiled = TilingPostprocess;
		};
		#pragma pack(pop)
	}
