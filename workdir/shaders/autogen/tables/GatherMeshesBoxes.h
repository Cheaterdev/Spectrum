#pragma once
#include "BoxInfo.h"
struct GatherMeshesBoxes_srv
{
	uint input_meshes; // StructuredBuffer<BoxInfo>
	uint visible_boxes; // StructuredBuffer<uint>
};
struct GatherMeshesBoxes_uav
{
	uint visibleMeshes; // AppendStructuredBuffer<uint>
	uint invisibleMeshes; // AppendStructuredBuffer<uint>
};
struct GatherMeshesBoxes
{
	GatherMeshesBoxes_srv srv;
	GatherMeshesBoxes_uav uav;
	StructuredBuffer<BoxInfo> GetInput_meshes() { return ResourceDescriptorHeap[srv.input_meshes]; }
	StructuredBuffer<uint> GetVisible_boxes() { return ResourceDescriptorHeap[srv.visible_boxes]; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return ResourceDescriptorHeap[uav.visibleMeshes]; }
	AppendStructuredBuffer<uint> GetInvisibleMeshes() { return ResourceDescriptorHeap[uav.invisibleMeshes]; }

};
 const GatherMeshesBoxes CreateGatherMeshesBoxes(GatherMeshesBoxes_srv srv,GatherMeshesBoxes_uav uav)
{
	const GatherMeshesBoxes result = {srv,uav
	};
	return result;
}
