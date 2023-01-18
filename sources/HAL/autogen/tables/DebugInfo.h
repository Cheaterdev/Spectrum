#pragma once
#include "DebugStruct.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugInfo
	{
		HLSL::RWStructuredBuffer<DebugStruct> debug;
		HLSL::RWStructuredBuffer<DebugStruct>& GetDebug() { return debug; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(debug);
		}
		struct Compiled
		{
			uint debug; // RWStructuredBuffer<DebugStruct>
		};
	};
	#pragma pack(pop)
}
