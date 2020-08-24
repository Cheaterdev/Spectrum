#pragma once
struct VoxelBlur_srv
{
	Texture2D<float4> tex_color;
};
struct VoxelBlur
{
	VoxelBlur_srv srv;
	Texture2D<float4> GetTex_color() { return srv.tex_color; }

};
 const VoxelBlur CreateVoxelBlur(VoxelBlur_srv srv)
{
	const VoxelBlur result = {srv
	};
	return result;
}
