#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelZero.h"
ConstantBuffer<VoxelZero_cb> cb_4_0:register(b0,space4);
RWTexture3D<float4> uav_4_0: register(u0, space4);
StructuredBuffer<int3> srv_4_0: register(t0, space4);
struct Pass_VoxelZero
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_VoxelZero> pass_VoxelZero: register( b2, space4);
const VoxelZero CreateVoxelZero()
{
	VoxelZero result;
	Pass_VoxelZero pass;
	result.cb = cb_4_0;
	result.uav.Target = uav_4_0;
	result.srv.params.tiles = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelZero voxelZero_global = CreateVoxelZero();
const VoxelZero GetVoxelZero(){ return voxelZero_global; }
#endif
