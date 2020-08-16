#pragma once
struct VoxelUpscale_srv
{
	Texture2D<float4> tex_downsampled;
	Texture2D<float4> tex_gi_prev;
	Texture2D<float> tex_depth_prev;
};
struct VoxelUpscale
{
	VoxelUpscale_srv srv;
	Texture2D<float4> GetTex_downsampled() { return srv.tex_downsampled; }
	Texture2D<float4> GetTex_gi_prev() { return srv.tex_gi_prev; }
	Texture2D<float> GetTex_depth_prev() { return srv.tex_depth_prev; }
};
 const VoxelUpscale CreateVoxelUpscale(VoxelUpscale_srv srv)
{
	const VoxelUpscale result = {srv
	};
	return result;
}
