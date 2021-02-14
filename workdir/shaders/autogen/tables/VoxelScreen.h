#pragma once
#include "GBuffer.h"
struct VoxelScreen_srv
{
	Texture3D<float4> voxels;
	TextureCube<float4> tex_cube;
	Texture2D<float4> prev_gi;
	Texture2D<float> prev_frames;
	Texture2D<float> prev_depth;
	GBuffer_srv gbuffer;
};
struct VoxelScreen
{
	VoxelScreen_srv srv;
	Texture3D<float4> GetVoxels() { return srv.voxels; }
	TextureCube<float4> GetTex_cube() { return srv.tex_cube; }
	Texture2D<float4> GetPrev_gi() { return srv.prev_gi; }
	Texture2D<float> GetPrev_frames() { return srv.prev_frames; }
	Texture2D<float> GetPrev_depth() { return srv.prev_depth; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const VoxelScreen CreateVoxelScreen(VoxelScreen_srv srv)
{
	const VoxelScreen result = {srv
	};
	return result;
}
