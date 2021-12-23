#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelCopy.h"
ConstantBuffer<VoxelCopy_cb> cb_5_0:register(b0,space5);
Texture3D<float4> srv_5_0[2]: register(t0, space5);
RWTexture3D<float4> uav_5_0[2]: register(u0, space5);
StructuredBuffer<int3> srv_5_2: register(t2, space5);
struct Pass_VoxelCopy
{
uint srv_0;
uint srv_2;
uint uav_0;
};
ConstantBuffer<Pass_VoxelCopy> pass_VoxelCopy: register( b2, space5);
const VoxelCopy CreateVoxelCopy()
{
	VoxelCopy result;
	result.cb = cb_5_0;
	result.srv.Source[0] = (pass_VoxelCopy.srv_0) + 0; 
	result.srv.Source[1] = (pass_VoxelCopy.srv_0) + 1; 
	result.uav.Target[0] = (pass_VoxelCopy.uav_0) + 0; 
	result.uav.Target[1] = (pass_VoxelCopy.uav_0) + 1; 
	result.srv.params.tiles = (pass_VoxelCopy.srv_2 );
	return result;
}
#ifndef NO_GLOBAL
static const VoxelCopy voxelCopy_global = CreateVoxelCopy();
const VoxelCopy GetVoxelCopy(){ return voxelCopy_global; }
#endif
