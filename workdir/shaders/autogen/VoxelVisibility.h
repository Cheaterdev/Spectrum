#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelVisibility.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelVisibility: register( b2, space5);
 ConstantBuffer<VoxelVisibility> CreateVoxelVisibility()
{
	return ResourceDescriptorHeap[pass_VoxelVisibility.offset];
}
#ifndef NO_GLOBAL
static const VoxelVisibility voxelVisibility_global = CreateVoxelVisibility();
const VoxelVisibility GetVoxelVisibility(){ return voxelVisibility_global; }
#endif
