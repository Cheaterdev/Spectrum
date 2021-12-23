#pragma once
#include "VoxelTilingParams.h"
struct VoxelMipMap_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelMipMap_srv
{
	uint SrcMip; // Texture3D<float4>
	VoxelTilingParams_srv params;
};
struct VoxelMipMap_uav
{
	uint OutMips[3]; // RWTexture3D<float4>
};
struct VoxelMipMap
{
	VoxelMipMap_cb cb;
	VoxelMipMap_srv srv;
	VoxelMipMap_uav uav;
	RWTexture3D<float4> GetOutMips(int i) { return ResourceDescriptorHeap[uav.OutMips[i]]; }
	Texture3D<float4> GetSrcMip() { return ResourceDescriptorHeap[srv.SrcMip]; }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelMipMap CreateVoxelMipMap(VoxelMipMap_cb cb,VoxelMipMap_srv srv,VoxelMipMap_uav uav)
{
	const VoxelMipMap result = {cb,srv,uav
	};
	return result;
}
