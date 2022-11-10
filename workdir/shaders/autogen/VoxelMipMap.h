#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelMipMap.h"
ConstantBuffer<VoxelMipMap> pass_VoxelMipMap: register( b2, space5);
const VoxelMipMap CreateVoxelMipMap()
{
	return pass_VoxelMipMap;
}
#ifndef NO_GLOBAL
static const VoxelMipMap voxelMipMap_global = CreateVoxelMipMap();
const VoxelMipMap GetVoxelMipMap(){ return voxelMipMap_global; }
#endif
