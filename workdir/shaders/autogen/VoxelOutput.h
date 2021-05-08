#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelOutput.h"
RWTexture2D<float4> uav_5_0: register(u0, space5);
RWTexture2D<float> uav_5_1: register(u1, space5);
struct Pass_VoxelOutput
{
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_VoxelOutput> pass_VoxelOutput: register( b2, space5);
const VoxelOutput CreateVoxelOutput()
{
	VoxelOutput result;
	Pass_VoxelOutput pass;
	result.uav.noise = uav_5_0;
	result.uav.frames = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelOutput voxelOutput_global = CreateVoxelOutput();
const VoxelOutput GetVoxelOutput(){ return voxelOutput_global; }
#endif
