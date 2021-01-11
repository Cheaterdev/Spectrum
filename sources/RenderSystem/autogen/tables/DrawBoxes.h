#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawBoxes
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<float4> vertices;
			DX12::HLSL::StructuredBuffer<BoxInfo> input_meshes;
		} &srv;
		struct UAV
		{
			DX12::HLSL::RWStructuredBuffer<uint> visible_meshes;
		} &uav;
		DX12::HLSL::StructuredBuffer<float4>& GetVertices() { return srv.vertices; }
		DX12::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return srv.input_meshes; }
		DX12::HLSL::RWStructuredBuffer<uint>& GetVisible_meshes() { return uav.visible_meshes; }
		DrawBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
