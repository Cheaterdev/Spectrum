#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
struct GatherPipelineGlobal_cb
{
	uint meshes_count;
};
struct GatherPipelineGlobal_srv
{
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;
};
struct GatherPipelineGlobal
{
	GatherPipelineGlobal_cb cb;
	GatherPipelineGlobal_srv srv;
	uint GetMeshes_count() { return cb.meshes_count; }
	StructuredBuffer<MeshCommandData> GetMeshes() { return srv.meshes; }
	StructuredBuffer<MaterialCommandData> GetMaterials() { return srv.materials; }
};
 const GatherPipelineGlobal CreateGatherPipelineGlobal(GatherPipelineGlobal_cb cb,GatherPipelineGlobal_srv srv)
{
	const GatherPipelineGlobal result = {cb,srv
	};
	return result;
}
