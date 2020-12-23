#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelVisibility.h"
Texture3D<uint> srv_4_0: register(t0, space4);
AppendStructuredBuffer<uint4> uav_4_0: register(u0, space4);
VoxelVisibility CreateVoxelVisibility()
{
	VoxelVisibility result;
	result.srv.visibility = srv_4_0;
	result.uav.visible_tiles = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelVisibility voxelVisibility_global = CreateVoxelVisibility();
const VoxelVisibility GetVoxelVisibility(){ return voxelVisibility_global; }
#endif
