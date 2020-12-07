#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<uint> meshes_count;
			Render::HLSL::Buffer<uint> commands;
		} &srv;
		Render::HLSL::StructuredBuffer<uint>& GetMeshes_count() { return srv.meshes_count; }
		Render::HLSL::Buffer<uint>& GetCommands() { return srv.commands; }
		GatherPipelineGlobal(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
