#pragma once
#include "VSLine.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		HLSL::StructuredBuffer<VSLine> vb;
		HLSL::StructuredBuffer<VSLine>& GetVb() { return vb; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vb);
		}
	};
	#pragma pack(pop)
}
