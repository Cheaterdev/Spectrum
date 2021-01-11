#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelScreen
	{
		struct SRV
		{
			DX12::HLSL::Texture3D<float4> voxels;
			DX12::HLSL::TextureCube<float4> tex_cube;
			GBuffer::SRV gbuffer;
		} &srv;
		DX12::HLSL::Texture3D<float4>& GetVoxels() { return srv.voxels; }
		DX12::HLSL::TextureCube<float4>& GetTex_cube() { return srv.tex_cube; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelScreen(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
