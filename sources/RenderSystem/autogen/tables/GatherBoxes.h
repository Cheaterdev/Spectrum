#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherBoxes
	{
		struct UAV
		{
			DX12::HLSL::AppendStructuredBuffer<BoxInfo> culledMeshes;
			DX12::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		} &uav;
		DX12::HLSL::AppendStructuredBuffer<BoxInfo>& GetCulledMeshes() { return uav.culledMeshes; }
		DX12::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return uav.visibleMeshes; }
		GatherBoxes(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
