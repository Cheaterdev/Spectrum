#pragma once
#include "GBuffer.h"
struct VoxelScreen
{
	uint voxels; // Texture3D<float4>
	uint tex_cube; // TextureCube<float4>
	uint prev_gi; // Texture2D<float4>
	uint prev_frames; // Texture2D<float>
	uint prev_depth; // Texture2D<float>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	Texture3D<float4> GetVoxels() { return ResourceDescriptorHeap[voxels]; }
	TextureCube<float4> GetTex_cube() { return ResourceDescriptorHeap[tex_cube]; }
	Texture2D<float4> GetPrev_gi() { return ResourceDescriptorHeap[prev_gi]; }
	Texture2D<float> GetPrev_frames() { return ResourceDescriptorHeap[prev_frames]; }
	Texture2D<float> GetPrev_depth() { return ResourceDescriptorHeap[prev_depth]; }
};
