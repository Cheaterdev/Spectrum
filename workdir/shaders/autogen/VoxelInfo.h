#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
ConstantBuffer<VoxelInfo_cb> cb_2_0:register(b0,space2);
VoxelInfo CreateVoxelInfo()
{
	VoxelInfo result;
	result.cb = cb_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
#endif
