#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelMipMap.h"
ConstantBuffer<VoxelMipMap_cb> cb_5_0:register(b0,space5);
Texture3D<float4> srv_5_0: register(t0, space5);
RWTexture3D<float4> uav_5_0[3]: register(u0, space5);
StructuredBuffer<int3> srv_5_1: register(t1, space5);
struct Pass_VoxelMipMap
{
uint srv_0;
uint srv_1;
uint uav_0;
};
ConstantBuffer<Pass_VoxelMipMap> pass_VoxelMipMap: register( b2, space5);
const VoxelMipMap CreateVoxelMipMap()
{
	VoxelMipMap result;
	result.cb = cb_5_0;
	result.srv.SrcMip = srv_5_0;
	result.uav.OutMips = uav_5_0;
	result.srv.params.tiles = srv_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelMipMap voxelMipMap_global = CreateVoxelMipMap();
const VoxelMipMap GetVoxelMipMap(){ return voxelMipMap_global; }
#endif
