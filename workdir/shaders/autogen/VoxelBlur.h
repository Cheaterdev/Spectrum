#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float4> srv_6_1: register(t1, space6);
Texture2D<float4> srv_6_2: register(t2, space6);
RWTexture2D<float4> uav_6_0: register(u0, space6);
RWTexture2D<float4> uav_6_1: register(u1, space6);
struct Pass_VoxelBlur
{
uint srv_0;
uint srv_1;
uint srv_2;
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_VoxelBlur> pass_VoxelBlur: register( b2, space6);
const VoxelBlur CreateVoxelBlur()
{
	VoxelBlur result;
	Pass_VoxelBlur pass;
	result.srv.noisy_output = srv_6_0;
	result.srv.hit_and_pdf = srv_6_1;
	result.srv.prev_result = srv_6_2;
	result.uav.screen_result = uav_6_0;
	result.uav.gi_result = uav_6_1;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
