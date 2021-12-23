#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Voxelization.h"
ConstantBuffer<Voxelization_cb> cb_6_0:register(b0,space6);
RWTexture3D<float4> uav_6_0: register(u0, space6);
RWTexture3D<float4> uav_6_1: register(u1, space6);
RWTexture3D<uint> uav_6_2: register(u2, space6);
struct Pass_Voxelization
{
uint uav_0;
uint uav_1;
uint uav_2;
};
ConstantBuffer<Pass_Voxelization> pass_Voxelization: register( b2, space6);
const Voxelization CreateVoxelization()
{
	Voxelization result;
	result.cb = cb_6_0;
	result.uav.albedo = (pass_Voxelization.uav_0 );
	result.uav.normals = (pass_Voxelization.uav_1 );
	result.uav.visibility = (pass_Voxelization.uav_2 );
	return result;
}
#ifndef NO_GLOBAL
static const Voxelization voxelization_global = CreateVoxelization();
const Voxelization GetVoxelization(){ return voxelization_global; }
#endif
