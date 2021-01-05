#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelCopy.h"
ConstantBuffer<VoxelCopy_cb> cb_4_0:register(b0,space4);
Texture3D<float4> srv_4_0[2]: register(t0, space4);
RWTexture3D<float4> uav_4_0[2]: register(u0, space4);
StructuredBuffer<int3> srv_4_2: register(t2, space4);
VoxelCopy CreateVoxelCopy()
{
	VoxelCopy result;
	result.cb = cb_4_0;
	result.srv.Source = srv_4_0;
	result.uav.Target = uav_4_0;
	result.srv.params.tiles = srv_4_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelCopy voxelCopy_global = CreateVoxelCopy();
const VoxelCopy GetVoxelCopy(){ return voxelCopy_global; }
#endif
