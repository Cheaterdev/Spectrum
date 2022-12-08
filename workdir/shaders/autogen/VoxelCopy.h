#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelCopy.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelCopy: register( b2, space5);
 ConstantBuffer<VoxelCopy> CreateVoxelCopy()
{
	return ResourceDescriptorHeap[pass_VoxelCopy.offset];
}
#ifndef NO_GLOBAL
static const VoxelCopy voxelCopy_global = CreateVoxelCopy();
const VoxelCopy GetVoxelCopy(){ return voxelCopy_global; }
#endif
