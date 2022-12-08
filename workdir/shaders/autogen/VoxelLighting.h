#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelLighting.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelLighting: register( b2, space5);
 ConstantBuffer<VoxelLighting> CreateVoxelLighting()
{
	return ResourceDescriptorHeap[pass_VoxelLighting.offset];
}
#ifndef NO_GLOBAL
static const VoxelLighting voxelLighting_global = CreateVoxelLighting();
const VoxelLighting GetVoxelLighting(){ return voxelLighting_global; }
#endif
