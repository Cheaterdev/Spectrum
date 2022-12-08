#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelDebug.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelDebug: register( b2, space5);
 ConstantBuffer<VoxelDebug> CreateVoxelDebug()
{
	return ResourceDescriptorHeap[pass_VoxelDebug.offset];
}
#ifndef NO_GLOBAL
static const VoxelDebug voxelDebug_global = CreateVoxelDebug();
const VoxelDebug GetVoxelDebug(){ return voxelDebug_global; }
#endif
