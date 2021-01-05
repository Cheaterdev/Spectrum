#pragma once
#include "VoxelTilingParams.h"
struct VoxelCopy_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelCopy_srv
{
	Texture3D<float4> Source[2];
	VoxelTilingParams_srv params;
};
struct VoxelCopy_uav
{
	RWTexture3D<float4> Target[2];
};
struct VoxelCopy
{
	VoxelCopy_cb cb;
	VoxelCopy_srv srv;
	VoxelCopy_uav uav;
	RWTexture3D<float4> GetTarget(int i) { return uav.Target[i]; }
	Texture3D<float4> GetSource(int i) { return srv.Source[i]; }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelCopy CreateVoxelCopy(VoxelCopy_cb cb,VoxelCopy_srv srv,VoxelCopy_uav uav)
{
	const VoxelCopy result = {cb,srv,uav
	};
	return result;
}
