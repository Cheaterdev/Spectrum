#pragma once
struct VoxelUpscale_srv
{
	uint tex_downsampled; // Texture2D<float4>
	uint tex_gi_prev; // Texture2D<float4>
	uint tex_depth_prev; // Texture2D<float>
};
struct VoxelUpscale
{
	VoxelUpscale_srv srv;
	Texture2D<float4> GetTex_downsampled() { return ResourceDescriptorHeap[srv.tex_downsampled]; }
	Texture2D<float4> GetTex_gi_prev() { return ResourceDescriptorHeap[srv.tex_gi_prev]; }
	Texture2D<float> GetTex_depth_prev() { return ResourceDescriptorHeap[srv.tex_depth_prev]; }

};
 const VoxelUpscale CreateVoxelUpscale(VoxelUpscale_srv srv)
{
	const VoxelUpscale result = {srv
	};
	return result;
}
