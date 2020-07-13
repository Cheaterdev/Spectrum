#pragma once
struct GatherPipelineGlobal_cb
{
	uint meshes_count;
};
struct GatherPipelineGlobal_srv
{
	Buffer<uint> commands;
};
struct GatherPipelineGlobal
{
	GatherPipelineGlobal_cb cb;
	GatherPipelineGlobal_srv srv;
	uint GetMeshes_count() { return cb.meshes_count; }
	Buffer<uint> GetCommands() { return srv.commands; }
};
 const GatherPipelineGlobal CreateGatherPipelineGlobal(GatherPipelineGlobal_cb cb,GatherPipelineGlobal_srv srv)
{
	const GatherPipelineGlobal result = {cb,srv
	};
	return result;
}
