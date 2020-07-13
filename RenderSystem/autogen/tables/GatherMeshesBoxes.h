#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherMeshesBoxes
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle input_meshes;
			Render::Handle visible_boxes;
		} &srv;
		struct UAV
		{
			Render::Handle visibleMeshes;
			Render::Handle invisibleMeshes;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetInput_meshes() { return srv.input_meshes; }
		Render::Handle& GetVisible_boxes() { return srv.visible_boxes; }
		Render::Handle& GetVisibleMeshes() { return uav.visibleMeshes; }
		Render::Handle& GetInvisibleMeshes() { return uav.invisibleMeshes; }
		GatherMeshesBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
