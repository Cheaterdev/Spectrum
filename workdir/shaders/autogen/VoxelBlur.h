#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
ConstantBuffer<VoxelBlur> pass_VoxelBlur: register( b2, space6);
const VoxelBlur CreateVoxelBlur()
{
	return pass_VoxelBlur;
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
