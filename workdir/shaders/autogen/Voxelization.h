#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Voxelization.h"
ConstantBuffer<Voxelization> pass_Voxelization: register( b2, space6);
const Voxelization CreateVoxelization()
{
	return pass_Voxelization;
}
#ifndef NO_GLOBAL
static const Voxelization voxelization_global = CreateVoxelization();
const Voxelization GetVoxelization(){ return voxelization_global; }
#endif
