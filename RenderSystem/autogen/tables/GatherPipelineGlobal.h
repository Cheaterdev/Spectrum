#pragma once
#include "MaterialCommandData.h"
#include "MeshCommandData.h"
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
			Render::Handle meshes;
			Render::Handle materials;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetMeshes_count() { return cb.meshes_count; }
		Render::Handle& GetMeshes() { return srv.meshes; }
		Render::Handle& GetMaterials() { return srv.materials; }
		GatherPipelineGlobal(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
