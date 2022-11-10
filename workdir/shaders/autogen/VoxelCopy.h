#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelCopy.h"
ConstantBuffer<VoxelCopy> pass_VoxelCopy: register( b2, space5);
const VoxelCopy CreateVoxelCopy()
{
	return pass_VoxelCopy;
}
#ifndef NO_GLOBAL
static const VoxelCopy voxelCopy_global = CreateVoxelCopy();
const VoxelCopy GetVoxelCopy(){ return voxelCopy_global; }
#endif
