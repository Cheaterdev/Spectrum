#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
#include "RaytraceInstanceInfo.h"
#include "node_data.h"
struct SceneData
{
	uint nodes; // StructuredBuffer<node_data>
	uint meshes; // StructuredBuffer<MeshCommandData>
	uint materials; // StructuredBuffer<MaterialCommandData>
	uint raytraceInstanceInfo; // StructuredBuffer<RaytraceInstanceInfo>
	StructuredBuffer<node_data> GetNodes() { return ResourceDescriptorHeap[nodes]; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return ResourceDescriptorHeap[meshes]; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return ResourceDescriptorHeap[materials]; }
	StructuredBuffer<RaytraceInstanceInfo> GetRaytraceInstanceInfo() { return ResourceDescriptorHeap[raytraceInstanceInfo]; }
};
