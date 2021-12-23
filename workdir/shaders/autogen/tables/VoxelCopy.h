#pragma once
#include "VoxelTilingParams.h"
struct VoxelCopy_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelCopy_srv
{
	uint Source[2]; // Texture3D<float4>
	VoxelTilingParams_srv params;
};
struct VoxelCopy_uav
{
	uint Target[2]; // RWTexture3D<float4>
};
struct VoxelCopy
{
	VoxelCopy_cb cb;
	VoxelCopy_srv srv;
	VoxelCopy_uav uav;
	RWTexture3D<float4> GetTarget(int i) { return ResourceDescriptorHeap[uav.Target[i]]; }
	Texture3D<float4> GetSource(int i) { return ResourceDescriptorHeap[srv.Source[i]]; }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelCopy CreateVoxelCopy(VoxelCopy_cb cb,VoxelCopy_srv srv,VoxelCopy_uav uav)
{
	const VoxelCopy result = {cb,srv,uav
	};
	return result;
}
