#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelBlur: register( b2, space6);
 ConstantBuffer<VoxelBlur> CreateVoxelBlur()
{
	return ResourceDescriptorHeap[pass_VoxelBlur.offset];
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
