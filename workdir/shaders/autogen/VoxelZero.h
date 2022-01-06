#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelZero.h"
ConstantBuffer<VoxelZero> pass_VoxelZero: register( b2, space5);
const VoxelZero CreateVoxelZero()
{
	return pass_VoxelZero;
}
#ifndef NO_GLOBAL
static const VoxelZero voxelZero_global = CreateVoxelZero();
const VoxelZero GetVoxelZero(){ return voxelZero_global; }
#endif
