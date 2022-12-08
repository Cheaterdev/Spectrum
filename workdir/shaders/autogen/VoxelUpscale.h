#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUpscale.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelUpscale: register( b2, space6);
 ConstantBuffer<VoxelUpscale> CreateVoxelUpscale()
{
	return ResourceDescriptorHeap[pass_VoxelUpscale.offset];
}
#ifndef NO_GLOBAL
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
#endif
