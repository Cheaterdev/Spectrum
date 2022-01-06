#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUpscale.h"
ConstantBuffer<VoxelUpscale> pass_VoxelUpscale: register( b2, space6);
const VoxelUpscale CreateVoxelUpscale()
{
	return pass_VoxelUpscale;
}
#ifndef NO_GLOBAL
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
#endif
