#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherBoxes
	{
		using CB = Empty;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle culledMeshes;
			Render::Handle visibleMeshes;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetCulledMeshes() { return uav.culledMeshes; }
		Render::Handle& GetVisibleMeshes() { return uav.visibleMeshes; }
		GatherBoxes(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}
