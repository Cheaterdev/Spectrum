#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherBoxes
	{
		struct UAV
		{
			Render::HLSL::AppendStructuredBuffer<BoxInfo> culledMeshes;
			Render::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		} &uav;
		Render::HLSL::AppendStructuredBuffer<BoxInfo>& GetCulledMeshes() { return uav.culledMeshes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return uav.visibleMeshes; }
		GatherBoxes(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
