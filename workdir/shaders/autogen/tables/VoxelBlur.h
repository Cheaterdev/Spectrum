#pragma once
struct VoxelBlur_srv
{
	Texture2D<float4> noisy_output;
	Texture2D<float4> hit_and_pdf;
	Texture2D<float4> prev_result;
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
	Texture2D<float4> GetHit_and_pdf() { return srv.hit_and_pdf; }
	Texture2D<float4> GetPrev_result() { return srv.prev_result; }
	RWTexture2D<float4> GetScreen_result() { return uav.screen_result; }
	RWTexture2D<float4> GetGi_result() { return uav.gi_result; }

};
 const VoxelBlur CreateVoxelBlur(VoxelBlur_srv srv,VoxelBlur_uav uav)
{
	const VoxelBlur result = {srv,uav
	};
	return result;
}
