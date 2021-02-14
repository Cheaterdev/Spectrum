#pragma once
#include "TilingParams.h"
struct VoxelBlur_srv
{
	Texture2D<float4> noisy_output;
	Texture2D<float4> prev_result;
	TilingParams_srv tiling;
};
struct VoxelBlur_uav
{
	RWTexture2D<float4> screen_result;
	RWTexture2D<float4> gi_result;
};
struct VoxelBlur
{
	VoxelBlur_srv srv;
	VoxelBlur_uav uav;
	Texture2D<float4> GetNoisy_output() { return srv.noisy_output; }
	Texture2D<float4> GetPrev_result() { return srv.prev_result; }
	RWTexture2D<float4> GetScreen_result() { return uav.screen_result; }
	RWTexture2D<float4> GetGi_result() { return uav.gi_result; }
	TilingParams GetTiling() { return CreateTilingParams(srv.tiling); }

};
 const VoxelBlur CreateVoxelBlur(VoxelBlur_srv srv,VoxelBlur_uav uav)
{
	const VoxelBlur result = {srv,uav
	};
	return result;
}
