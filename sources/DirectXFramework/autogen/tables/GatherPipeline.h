#pragma once
#include "CommandData.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipeline
	{
		uint4 pip_ids[2];
		Render::HLSL::AppendStructuredBuffer<CommandData> commands[8];
		uint4* GetPip_ids() { return pip_ids; }
		Render::HLSL::AppendStructuredBuffer<CommandData>* GetCommands() { return commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pip_ids);
			compiler.compile(commands);
		}
	};
	#pragma pack(pop)
}
