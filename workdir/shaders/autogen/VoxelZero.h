#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelZero.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelZero: register( b2, space5);
 ConstantBuffer<VoxelZero> CreateVoxelZero()
{
	return ResourceDescriptorHeap[pass_VoxelZero.offset];
}
#ifndef NO_GLOBAL
static const VoxelZero voxelZero_global = CreateVoxelZero();
const VoxelZero GetVoxelZero(){ return voxelZero_global; }
#endif
