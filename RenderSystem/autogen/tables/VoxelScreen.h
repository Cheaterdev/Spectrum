#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelScreen
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle voxels;
			Render::Handle tex_cube;
			GBuffer::SRV gbuffer;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetVoxels() { return srv.voxels; }
		Render::Handle& GetTex_cube() { return srv.tex_cube; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelScreen(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
