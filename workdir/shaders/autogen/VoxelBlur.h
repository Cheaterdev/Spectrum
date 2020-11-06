#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
Texture2D<float4> srv_5_0: register(t0, space5);
VoxelBlur CreateVoxelBlur()
{
	VoxelBlur result;
	result.srv.tex_color = srv_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
