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
			Render::HLSL::Texture2D<float4> prev_gi;
			Render::HLSL::Texture2D<float> prev_frames;
			Render::HLSL::Texture2D<float> prev_depth;
			GBuffer::SRV gbuffer;
		} &srv;
		Render::HLSL::Texture3D<float4>& GetVoxels() { return srv.voxels; }
		Render::HLSL::TextureCube<float4>& GetTex_cube() { return srv.tex_cube; }
		Render::HLSL::Texture2D<float4>& GetPrev_gi() { return srv.prev_gi; }
		Render::HLSL::Texture2D<float>& GetPrev_frames() { return srv.prev_frames; }
		Render::HLSL::Texture2D<float>& GetPrev_depth() { return srv.prev_depth; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		VoxelScreen(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
