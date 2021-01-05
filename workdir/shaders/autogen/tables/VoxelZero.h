#pragma once
#include "VoxelTilingParams.h"
struct VoxelZero_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelZero_srv
{
	VoxelTilingParams_srv params;
};
struct VoxelZero_uav
{
	RWTexture3D<float4> Target;
};
struct VoxelZero
{
	VoxelZero_cb cb;
	VoxelZero_srv srv;
	VoxelZero_uav uav;
	RWTexture3D<float4> GetTarget() { return uav.Target; }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelZero CreateVoxelZero(VoxelZero_cb cb,VoxelZero_srv srv,VoxelZero_uav uav)
{
	const VoxelZero result = {cb,srv,uav
	};
	return result;
}
