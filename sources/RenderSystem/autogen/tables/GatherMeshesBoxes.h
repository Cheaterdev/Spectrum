#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherMeshesBoxes
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<BoxInfo> input_meshes;
			DX12::HLSL::StructuredBuffer<uint> visible_boxes;
		} &srv;
		struct UAV
		{
			DX12::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
			DX12::HLSL::AppendStructuredBuffer<uint> invisibleMeshes;
		} &uav;
		DX12::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return srv.input_meshes; }
		DX12::HLSL::StructuredBuffer<uint>& GetVisible_boxes() { return srv.visible_boxes; }
		DX12::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return uav.visibleMeshes; }
		DX12::HLSL::AppendStructuredBuffer<uint>& GetInvisibleMeshes() { return uav.invisibleMeshes; }
		GatherMeshesBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
