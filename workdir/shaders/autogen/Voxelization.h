#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Voxelization.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Voxelization: register( b2, space6);
 ConstantBuffer<Voxelization> CreateVoxelization()
{
	return ResourceDescriptorHeap[pass_Voxelization.offset];
}
#ifndef NO_GLOBAL
static const Voxelization voxelization_global = CreateVoxelization();
const Voxelization GetVoxelization(){ return voxelization_global; }
#endif
