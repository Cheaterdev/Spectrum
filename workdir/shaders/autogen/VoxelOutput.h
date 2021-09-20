#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelOutput.h"
RWTexture2D<float4> uav_6_0: register(u0, space6);
RWTexture2D<float> uav_6_1: register(u1, space6);
RWTexture2D<float4> uav_6_2: register(u2, space6);
struct Pass_VoxelOutput
{
uint uav_0;
uint uav_1;
uint uav_2;
};
ConstantBuffer<Pass_VoxelOutput> pass_VoxelOutput: register( b2, space6);
const VoxelOutput CreateVoxelOutput()
{
	VoxelOutput result;
	result.uav.noise =  uav_6_0;
	result.uav.frames = uav_6_1;
	result.uav.DirAndPdf = uav_6_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelOutput voxelOutput_global = CreateVoxelOutput();
const VoxelOutput GetVoxelOutput(){ return voxelOutput_global; }
#endif
