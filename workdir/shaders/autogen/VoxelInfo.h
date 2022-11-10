#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
ConstantBuffer<VoxelInfo> pass_VoxelInfo: register( b2, space4);
const VoxelInfo CreateVoxelInfo()
{
	return pass_VoxelInfo;
}
#ifndef NO_GLOBAL
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
#endif
