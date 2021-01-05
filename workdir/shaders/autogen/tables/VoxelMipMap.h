#pragma once
#include "VoxelTilingParams.h"
struct VoxelMipMap_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelMipMap_srv
{
	Texture3D<float4> SrcMip;
	VoxelTilingParams_srv params;
};
struct VoxelMipMap_uav
{
	RWTexture3D<float4> OutMips[3];
};
struct VoxelMipMap
{
	VoxelMipMap_cb cb;
	VoxelMipMap_srv srv;
	VoxelMipMap_uav uav;
	RWTexture3D<float4> GetOutMips(int i) { return uav.OutMips[i]; }
	Texture3D<float4> GetSrcMip() { return srv.SrcMip; }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelMipMap CreateVoxelMipMap(VoxelMipMap_cb cb,VoxelMipMap_srv srv,VoxelMipMap_uav uav)
{
	const VoxelMipMap result = {cb,srv,uav
	};
	return result;
}
