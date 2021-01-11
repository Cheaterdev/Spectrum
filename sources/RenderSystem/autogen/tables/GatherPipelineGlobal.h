#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<uint> meshes_count;
			DX12::HLSL::Buffer<uint> commands;
		} &srv;
		DX12::HLSL::StructuredBuffer<uint>& GetMeshes_count() { return srv.meshes_count; }
		DX12::HLSL::Buffer<uint>& GetCommands() { return srv.commands; }
		GatherPipelineGlobal(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
