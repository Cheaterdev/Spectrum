#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherBoxes
	{
		HLSL::AppendStructuredBuffer<BoxInfo> culledMeshes;
		HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		HLSL::AppendStructuredBuffer<BoxInfo>& GetCulledMeshes() { return culledMeshes; }
		HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return visibleMeshes; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(culledMeshes);
			compiler.compile(visibleMeshes);
		}
	};
	#pragma pack(pop)
}