#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelMipMap.h"
ConstantBuffer<VoxelMipMap_cb> cb_3_0:register(b0,space3);
Texture3D<float4> srv_3_0: register(t0, space3);
StructuredBuffer<int3> srv_3_1: register(t1, space3);
RWTexture3D<float4> uav_3_0[3]: register(u0, space3);
VoxelMipMap CreateVoxelMipMap()
{
	VoxelMipMap result;
	result.cb = cb_3_0;
	result.srv.SrcMip = srv_3_0;
	result.srv.visibility = srv_3_1;
	result.uav.OutMips = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelMipMap voxelMipMap_global = CreateVoxelMipMap();
const VoxelMipMap GetVoxelMipMap(){ return voxelMipMap_global; }
#endif
