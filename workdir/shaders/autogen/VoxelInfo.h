#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelInfo.h"
ConstantBuffer<VoxelInfo_cb> cb_4_0:register(b0,space4);
struct Pass_VoxelInfo
{
};
ConstantBuffer<Pass_VoxelInfo> pass_VoxelInfo: register( b2, space4);
const VoxelInfo CreateVoxelInfo()
{
	VoxelInfo result;
	Pass_VoxelInfo pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelInfo voxelInfo_global = CreateVoxelInfo();
const VoxelInfo GetVoxelInfo(){ return voxelInfo_global; }
#endif
