#pragma once
struct VoxelBlur_srv
{
	uint noisy_output; // Texture2D<float4>
	uint hit_and_pdf; // Texture2D<float4>
	uint prev_result; // Texture2D<float4>
};
struct VoxelBlur_uav
{
	uint screen_result; // RWTexture2D<float4>
	uint gi_result; // RWTexture2D<float4>
};
struct VoxelBlur
{
	VoxelBlur_srv srv;
	VoxelBlur_uav uav;
	Texture2D<float4> GetNoisy_output() { return ResourceDescriptorHeap[srv.noisy_output]; }
	Texture2D<float4> GetHit_and_pdf() { return ResourceDescriptorHeap[srv.hit_and_pdf]; }
	Texture2D<float4> GetPrev_result() { return ResourceDescriptorHeap[srv.prev_result]; }
	RWTexture2D<float4> GetScreen_result() { return ResourceDescriptorHeap[uav.screen_result]; }
	RWTexture2D<float4> GetGi_result() { return ResourceDescriptorHeap[uav.gi_result]; }

};
 const VoxelBlur CreateVoxelBlur(VoxelBlur_srv srv,VoxelBlur_uav uav)
{
	const VoxelBlur result = {srv,uav
	};
	return result;
}
