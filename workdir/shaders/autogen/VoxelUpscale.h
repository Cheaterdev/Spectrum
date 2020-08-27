#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUpscale.h"
Texture2D<float4> srv_4_0: register(t0, space4);
Texture2D<float4> srv_4_1: register(t1, space4);
Texture2D<float> srv_4_2: register(t2, space4);
VoxelUpscale CreateVoxelUpscale()
{
	VoxelUpscale result;
	result.srv.tex_downsampled = srv_4_0;
	result.srv.tex_gi_prev = srv_4_1;
	result.srv.tex_depth_prev = srv_4_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
#endif
