#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		struct CB
		{
			uint meshes_count;
		} &cb;
		struct SRV
		{
			Render::Handle commands;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetMeshes_count() { return cb.meshes_count; }
		Render::Handle& GetCommands() { return srv.commands; }
		GatherPipelineGlobal(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
