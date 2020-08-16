#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Voxelization.h"
ConstantBuffer<Voxelization_cb> cb_5_0:register(b0,space5);
RWTexture3D<float4> uav_5_0: register(u0, space5);
RWTexture3D<float4> uav_5_1: register(u1, space5);
RWByteAddressBuffer uav_5_2: register(u2, space5);
Voxelization CreateVoxelization()
{
	Voxelization result;
	result.cb = cb_5_0;
	result.uav.albedo = uav_5_0;
	result.uav.normals = uav_5_1;
	result.uav.visibility = uav_5_2;
	return result;
}
static const Voxelization voxelization_global = CreateVoxelization();
const Voxelization GetVoxelization(){ return voxelization_global; }
