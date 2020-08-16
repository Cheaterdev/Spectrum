#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
ConstantBuffer<VoxelInfo_cb> cb_3_0:register(b0,space3);
VoxelInfo CreateVoxelInfo()
{
	VoxelInfo result;
	result.cb = cb_3_0;
	return result;
}
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
