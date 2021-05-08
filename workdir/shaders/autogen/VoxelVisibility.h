#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelVisibility.h"
Texture3D<uint> srv_4_0: register(t0, space4);
AppendStructuredBuffer<uint4> uav_4_0: register(u0, space4);
struct Pass_VoxelVisibility
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_VoxelVisibility> pass_VoxelVisibility: register( b2, space4);
const VoxelVisibility CreateVoxelVisibility()
{
	VoxelVisibility result;
	Pass_VoxelVisibility pass;
	result.srv.visibility = srv_4_0;
	result.uav.visible_tiles = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelVisibility voxelVisibility_global = CreateVoxelVisibility();
const VoxelVisibility GetVoxelVisibility(){ return voxelVisibility_global; }
#endif
