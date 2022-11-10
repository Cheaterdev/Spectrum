#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelOutput.h"
ConstantBuffer<VoxelOutput> pass_VoxelOutput: register( b2, space6);
const VoxelOutput CreateVoxelOutput()
{
	return pass_VoxelOutput;
}
#ifndef NO_GLOBAL
static const VoxelOutput voxelOutput_global = CreateVoxelOutput();
const VoxelOutput GetVoxelOutput(){ return voxelOutput_global; }
#endif
