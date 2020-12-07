#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelScreen
	{
		struct SRV
		{
			Render::HLSL::Texture3D<float4> voxels;
			Render::HLSL::TextureCube<float4> tex_cube;
			GBuffer::SRV gbuffer;
		} &srv;
		Render::HLSL::Texture3D<float4>& GetVoxels() { return srv.voxels; }
		Render::HLSL::TextureCube<float4>& GetTex_cube() { return srv.tex_cube; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelScreen(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
