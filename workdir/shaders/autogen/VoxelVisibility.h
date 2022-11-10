#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelVisibility.h"
ConstantBuffer<VoxelVisibility> pass_VoxelVisibility: register( b2, space5);
const VoxelVisibility CreateVoxelVisibility()
{
	return pass_VoxelVisibility;
}
#ifndef NO_GLOBAL
static const VoxelVisibility voxelVisibility_global = CreateVoxelVisibility();
const VoxelVisibility GetVoxelVisibility(){ return voxelVisibility_global; }
#endif
