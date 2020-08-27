#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelBlur.h"
Texture2D<float4> srv_4_0: register(t0, space4);
VoxelBlur CreateVoxelBlur()
{
	VoxelBlur result;
	result.srv.tex_color = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelBlur voxelBlur_global = CreateVoxelBlur();
const VoxelBlur GetVoxelBlur(){ return voxelBlur_global; }
#endif
