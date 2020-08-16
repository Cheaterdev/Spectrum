#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawBoxes
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle vertices;
			Render::Handle input_meshes;
		} &srv;
		struct UAV
		{
			Render::Handle visible_meshes;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetVertices() { return srv.vertices; }
		Render::Handle& GetInput_meshes() { return srv.input_meshes; }
		Render::Handle& GetVisible_meshes() { return uav.visible_meshes; }
		DrawBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
