#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherBoxes
	{
		Render::HLSL::AppendStructuredBuffer<BoxInfo> culledMeshes;
		Render::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		Render::HLSL::AppendStructuredBuffer<BoxInfo>& GetCulledMeshes() { return culledMeshes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return visibleMeshes; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(culledMeshes);
			compiler.compile(visibleMeshes);
		}
	};
	#pragma pack(pop)
}
