#pragma once
#include "DebugStruct.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugInfo
	{
		Render::HLSL::RWStructuredBuffer<DebugStruct> debug;
		Render::HLSL::RWStructuredBuffer<DebugStruct>& GetDebug() { return debug; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(debug);
		}
	};
	#pragma pack(pop)
}
