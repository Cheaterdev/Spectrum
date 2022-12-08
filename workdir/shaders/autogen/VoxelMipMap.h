#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelMipMap.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelMipMap: register( b2, space5);
 ConstantBuffer<VoxelMipMap> CreateVoxelMipMap()
{
	return ResourceDescriptorHeap[pass_VoxelMipMap.offset];
}
#ifndef NO_GLOBAL
static const VoxelMipMap voxelMipMap_global = CreateVoxelMipMap();
const VoxelMipMap GetVoxelMipMap(){ return voxelMipMap_global; }
#endif
