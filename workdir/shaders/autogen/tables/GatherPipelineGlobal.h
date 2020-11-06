#pragma once
struct GatherPipelineGlobal_srv
{
	StructuredBuffer<uint> meshes_count;
	Buffer<uint> commands;
};
struct GatherPipelineGlobal
{
	GatherPipelineGlobal_srv srv;
	StructuredBuffer<uint> GetMeshes_count() { return srv.meshes_count; }
	Buffer<uint> GetCommands() { return srv.commands; }

};
 const GatherPipelineGlobal CreateGatherPipelineGlobal(GatherPipelineGlobal_srv srv)
{
	const GatherPipelineGlobal result = {srv
	};
	return result;
}
