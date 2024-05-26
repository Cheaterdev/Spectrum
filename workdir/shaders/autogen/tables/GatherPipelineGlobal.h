#pragma once
#include "sig_hlsl.hlsl"
struct GatherPipelineGlobal
{
	uint meshes_count; // StructuredBuffer<uint>
	uint commands; // Buffer<uint>
	StructuredBuffer<uint> GetMeshes_count() { return ResourceDescriptorHeap[meshes_count]; }
	Buffer<uint> GetCommands() { return ResourceDescriptorHeap[commands]; }
};
