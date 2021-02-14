#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float4> srv_5_1: register(t1, space5);
RWTexture2D<float4> uav_5_0: register(u0, space5);
RWTexture2D<float4> uav_5_1: register(u1, space5);
StructuredBuffer<uint2> srv_5_2: register(t2, space5);
VoxelBlur CreateVoxelBlur()
{
	VoxelBlur result;
	result.srv.noisy_output = srv_5_0;
	result.srv.prev_result = srv_5_1;
	result.uav.screen_result = uav_5_0;
	result.uav.gi_result = uav_5_1;
	result.srv.tiling.tiles = srv_5_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
