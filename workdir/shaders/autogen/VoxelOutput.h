#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelOutput.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelOutput: register( b2, space6);
 ConstantBuffer<VoxelOutput> CreateVoxelOutput()
{
	return ResourceDescriptorHeap[pass_VoxelOutput.offset];
}
#ifndef NO_GLOBAL
static const VoxelOutput voxelOutput_global = CreateVoxelOutput();
const VoxelOutput GetVoxelOutput(){ return voxelOutput_global; }
#endif
