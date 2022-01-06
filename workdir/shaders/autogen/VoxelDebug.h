#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelDebug.h"
ConstantBuffer<VoxelDebug> pass_VoxelDebug: register( b2, space5);
const VoxelDebug CreateVoxelDebug()
{
	return pass_VoxelDebug;
}
#ifndef NO_GLOBAL
static const VoxelDebug voxelDebug_global = CreateVoxelDebug();
const VoxelDebug GetVoxelDebug(){ return voxelDebug_global; }
#endif
