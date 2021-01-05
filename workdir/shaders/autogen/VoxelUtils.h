#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUtils.h"
ConstantBuffer<VoxelUtils_cb> cb_4_0:register(b0,space4);
Texture3D<float4> srv_4_0[2]: register(t0, space4);
StructuredBuffer<int3> srv_4_2: register(t2, space4);
RWTexture3D<float4> uav_4_0[2]: register(u0, space4);
VoxelUtils CreateVoxelUtils()
{
	VoxelUtils result;
	result.cb = cb_4_0;
	result.srv.Source = srv_4_0;
	result.srv.visibility = srv_4_2;
	result.uav.Target = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelUtils voxelUtils_global = CreateVoxelUtils();
const VoxelUtils GetVoxelUtils(){ return voxelUtils_global; }
#endif
