#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelMipMap.h"
ConstantBuffer<VoxelMipMap_cb> cb_4_0:register(b0,space4);
Texture3D<float4> srv_4_0: register(t0, space4);
RWTexture3D<float4> uav_4_0[3]: register(u0, space4);
StructuredBuffer<int3> srv_4_1: register(t1, space4);
VoxelMipMap CreateVoxelMipMap()
{
	VoxelMipMap result;
	result.cb = cb_4_0;
	result.srv.SrcMip = srv_4_0;
	result.uav.OutMips = uav_4_0;
	result.srv.params.tiles = srv_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelMipMap voxelMipMap_global = CreateVoxelMipMap();
const VoxelMipMap GetVoxelMipMap(){ return voxelMipMap_global; }
#endif
