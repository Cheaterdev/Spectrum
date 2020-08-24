#pragma once
#include "BoxInfo.h"
struct DrawBoxes_srv
{
	StructuredBuffer<float4> vertices;
	StructuredBuffer<BoxInfo> input_meshes;
};
struct DrawBoxes_uav
{
	RWStructuredBuffer<uint> visible_meshes;
};
struct DrawBoxes
{
	DrawBoxes_srv srv;
	DrawBoxes_uav uav;
	StructuredBuffer<float4> GetVertices() { return srv.vertices; }
	StructuredBuffer<BoxInfo> GetInput_meshes() { return srv.input_meshes; }
	RWStructuredBuffer<uint> GetVisible_meshes() { return uav.visible_meshes; }

};
 const DrawBoxes CreateDrawBoxes(DrawBoxes_srv srv,DrawBoxes_uav uav)
{
	const DrawBoxes result = {srv,uav
	};
	return result;
}
