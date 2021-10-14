#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawBoxes
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<float4> vertices;
			Render::HLSL::StructuredBuffer<BoxInfo> input_meshes;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWStructuredBuffer<uint> visible_meshes;
		} &uav;
		Render::HLSL::StructuredBuffer<float4>& GetVertices() { return srv.vertices; }
		Render::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return srv.input_meshes; }
		Render::HLSL::RWStructuredBuffer<uint>& GetVisible_meshes() { return uav.visible_meshes; }
		DrawBoxes(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
