#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelScreen.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_VoxelScreen: register( b2, space5);
 ConstantBuffer<VoxelScreen> CreateVoxelScreen()
{
	return ResourceDescriptorHeap[pass_VoxelScreen.offset];
}
#ifndef NO_GLOBAL
static const VoxelScreen voxelScreen_global = CreateVoxelScreen();
const VoxelScreen GetVoxelScreen(){ return voxelScreen_global; }
#endif
