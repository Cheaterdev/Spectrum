#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelScreen.h"
ConstantBuffer<VoxelScreen> pass_VoxelScreen: register( b2, space5);
const VoxelScreen CreateVoxelScreen()
{
	return pass_VoxelScreen;
}
#ifndef NO_GLOBAL
static const VoxelScreen voxelScreen_global = CreateVoxelScreen();
const VoxelScreen GetVoxelScreen(){ return voxelScreen_global; }
#endif
