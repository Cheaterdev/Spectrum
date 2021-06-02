#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelZero.h"
ConstantBuffer<VoxelZero_cb> cb_5_0:register(b0,space5);
RWTexture3D<float4> uav_5_0: register(u0, space5);
StructuredBuffer<int3> srv_5_0: register(t0, space5);
struct Pass_VoxelZero
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_VoxelZero> pass_VoxelZero: register( b2, space5);
const VoxelZero CreateVoxelZero()
{
	VoxelZero result;
	result.cb = cb_5_0;
	result.uav.Target = uav_5_0;
	result.srv.params.tiles = srv_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelZero voxelZero_global = CreateVoxelZero();
const VoxelZero GetVoxelZero(){ return voxelZero_global; }
#endif
