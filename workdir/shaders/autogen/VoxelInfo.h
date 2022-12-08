#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelInfo: register( b2, space4);
 ConstantBuffer<VoxelInfo> CreateVoxelInfo()
{
	return ResourceDescriptorHeap[pass_VoxelInfo.offset];
}
#ifndef NO_GLOBAL
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
#endif
