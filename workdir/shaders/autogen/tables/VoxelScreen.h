#pragma once
#include "GBuffer.h"
struct VoxelScreen_srv
{
	Texture3D<float4> voxels;
	TextureCube<float4> tex_cube;
	GBuffer_srv gbuffer;
};
struct VoxelScreen
{
	VoxelScreen_srv srv;
	Texture3D<float4> GetVoxels() { return srv.voxels; }
	TextureCube<float4> GetTex_cube() { return srv.tex_cube; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }
};
 const VoxelScreen CreateVoxelScreen(VoxelScreen_srv srv)
{
	const VoxelScreen result = {srv
	};
	return result;
}
