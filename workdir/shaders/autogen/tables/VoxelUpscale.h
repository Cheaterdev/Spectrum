#pragma once
struct VoxelUpscale
{
	uint tex_downsampled; // Texture2D<float4>
	uint tex_gi_prev; // Texture2D<float4>
	uint tex_depth_prev; // Texture2D<float>
	Texture2D<float4> GetTex_downsampled() { return ResourceDescriptorHeap[tex_downsampled]; }
	Texture2D<float4> GetTex_gi_prev() { return ResourceDescriptorHeap[tex_gi_prev]; }
	Texture2D<float> GetTex_depth_prev() { return ResourceDescriptorHeap[tex_depth_prev]; }
};
