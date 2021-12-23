#pragma once
#include "BoxInfo.h"
struct DrawBoxes_srv
{
	uint vertices; // StructuredBuffer<float4>
	uint input_meshes; // StructuredBuffer<BoxInfo>
};
struct DrawBoxes_uav
{
	uint visible_meshes; // RWStructuredBuffer<uint>
};
struct DrawBoxes
{
	DrawBoxes_srv srv;
	DrawBoxes_uav uav;
	StructuredBuffer<float4> GetVertices() { return ResourceDescriptorHeap[srv.vertices]; }
	StructuredBuffer<BoxInfo> GetInput_meshes() { return ResourceDescriptorHeap[srv.input_meshes]; }
	RWStructuredBuffer<uint> GetVisible_meshes() { return ResourceDescriptorHeap[uav.visible_meshes]; }

};
 const DrawBoxes CreateDrawBoxes(DrawBoxes_srv srv,DrawBoxes_uav uav)
{
	const DrawBoxes result = {srv,uav
	};
	return result;
}
