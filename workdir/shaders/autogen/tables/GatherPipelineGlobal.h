#pragma once
struct GatherPipelineGlobal_srv
{
	uint meshes_count; // StructuredBuffer<uint>
	uint commands; // Buffer<uint>
};
struct GatherPipelineGlobal
{
	GatherPipelineGlobal_srv srv;
	StructuredBuffer<uint> GetMeshes_count() { return ResourceDescriptorHeap[srv.meshes_count]; }
	Buffer<uint> GetCommands() { return ResourceDescriptorHeap[srv.commands]; }

};
 const GatherPipelineGlobal CreateGatherPipelineGlobal(GatherPipelineGlobal_srv srv)
{
	const GatherPipelineGlobal result = {srv
	};
	return result;
}
