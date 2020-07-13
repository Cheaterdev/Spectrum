#pragma once
#include "BoxInfo.h"
struct GatherMeshesBoxes_srv
{
	StructuredBuffer<BoxInfo> input_meshes;
	StructuredBuffer<uint> visible_boxes;
};
struct GatherMeshesBoxes_uav
{
	AppendStructuredBuffer<uint> visibleMeshes;
	AppendStructuredBuffer<uint> invisibleMeshes;
};
struct GatherMeshesBoxes
{
	GatherMeshesBoxes_srv srv;
	GatherMeshesBoxes_uav uav;
	StructuredBuffer<BoxInfo> GetInput_meshes() { return srv.input_meshes; }
	StructuredBuffer<uint> GetVisible_boxes() { return srv.visible_boxes; }
	AppendStructuredBuffer<uint> GetVisibleMeshes() { return uav.visibleMeshes; }
	AppendStructuredBuffer<uint> GetInvisibleMeshes() { return uav.invisibleMeshes; }
};
 const GatherMeshesBoxes CreateGatherMeshesBoxes(GatherMeshesBoxes_srv srv,GatherMeshesBoxes_uav uav)
{
	const GatherMeshesBoxes result = {srv,uav
	};
	return result;
}
