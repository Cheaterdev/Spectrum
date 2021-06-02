#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelVisibility.h"
Texture3D<uint> srv_5_0: register(t0, space5);
AppendStructuredBuffer<uint4> uav_5_0: register(u0, space5);
struct Pass_VoxelVisibility
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_VoxelVisibility> pass_VoxelVisibility: register( b2, space5);
const VoxelVisibility CreateVoxelVisibility()
{
	VoxelVisibility result;
	result.srv.visibility = srv_5_0;
	result.uav.visible_tiles = uav_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelVisibility voxelVisibility_global = CreateVoxelVisibility();
const VoxelVisibility GetVoxelVisibility(){ return voxelVisibility_global; }
#endif
