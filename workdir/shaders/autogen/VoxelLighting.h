#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelLighting.h"
ConstantBuffer<VoxelLighting> pass_VoxelLighting: register( b2, space5);
const VoxelLighting CreateVoxelLighting()
{
	return pass_VoxelLighting;
}
#ifndef NO_GLOBAL
static const VoxelLighting voxelLighting_global = CreateVoxelLighting();
const VoxelLighting GetVoxelLighting(){ return voxelLighting_global; }
#endif
