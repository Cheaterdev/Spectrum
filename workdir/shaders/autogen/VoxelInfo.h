#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
ConstantBuffer<VoxelInfo_cb> cb_3_0:register(b0,space3);
struct Pass_VoxelInfo
{
};
ConstantBuffer<Pass_VoxelInfo> pass_VoxelInfo: register( b2, space3);
const VoxelInfo CreateVoxelInfo()
{
	VoxelInfo result;
	Pass_VoxelInfo pass;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
#endif
