#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle meshes_count;
			Render::Handle commands;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetMeshes_count() { return srv.meshes_count; }
		Render::Handle& GetCommands() { return srv.commands; }
		GatherPipelineGlobal(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
