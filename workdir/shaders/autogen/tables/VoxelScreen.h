#pragma once
#include "GBuffer.h"
struct VoxelScreen_srv
{
	uint voxels; // Texture3D<float4>
	uint tex_cube; // TextureCube<float4>
	uint prev_gi; // Texture2D<float4>
	uint prev_frames; // Texture2D<float>
	uint prev_depth; // Texture2D<float>
	GBuffer_srv gbuffer;
};
struct VoxelScreen
{
	VoxelScreen_srv srv;
	Texture3D<float4> GetVoxels() { return ResourceDescriptorHeap[srv.voxels]; }
	TextureCube<float4> GetTex_cube() { return ResourceDescriptorHeap[srv.tex_cube]; }
	Texture2D<float4> GetPrev_gi() { return ResourceDescriptorHeap[srv.prev_gi]; }
	Texture2D<float> GetPrev_frames() { return ResourceDescriptorHeap[srv.prev_frames]; }
	Texture2D<float> GetPrev_depth() { return ResourceDescriptorHeap[srv.prev_depth]; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const VoxelScreen CreateVoxelScreen(VoxelScreen_srv srv)
{
	const VoxelScreen result = {srv
	};
	return result;
}
