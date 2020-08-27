#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Voxelization.h"
ConstantBuffer<Voxelization_cb> cb_4_0:register(b0,space4);
RWTexture3D<float4> uav_4_0: register(u0, space4);
RWTexture3D<float4> uav_4_1: register(u1, space4);
RWByteAddressBuffer uav_4_2: register(u2, space4);
Voxelization CreateVoxelization()
{
	Voxelization result;
	result.cb = cb_4_0;
	result.uav.albedo = uav_4_0;
	result.uav.normals = uav_4_1;
	result.uav.visibility = uav_4_2;
	return result;
}
#ifndef NO_GLOBAL
static const Voxelization voxelization_global = CreateVoxelization();
const Voxelization GetVoxelization(){ return voxelization_global; }
#endif
