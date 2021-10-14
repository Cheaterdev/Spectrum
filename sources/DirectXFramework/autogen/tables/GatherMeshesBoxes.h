#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherMeshesBoxes
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<BoxInfo> input_meshes;
			Render::HLSL::StructuredBuffer<uint> visible_boxes;
		} &srv;
		struct UAV
		{
			Render::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
			Render::HLSL::AppendStructuredBuffer<uint> invisibleMeshes;
		} &uav;
		Render::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return srv.input_meshes; }
		Render::HLSL::StructuredBuffer<uint>& GetVisible_boxes() { return srv.visible_boxes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return uav.visibleMeshes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetInvisibleMeshes() { return uav.invisibleMeshes; }
		GatherMeshesBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
