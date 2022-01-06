#pragma once
#include "VSLine.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct LineRender
	{
		Render::HLSL::StructuredBuffer<VSLine> vb;
		Render::HLSL::StructuredBuffer<VSLine>& GetVb() { return vb; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vb);
		}
	};
	#pragma pack(pop)
}
