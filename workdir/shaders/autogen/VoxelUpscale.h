#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUpscale.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float4> srv_5_1: register(t1, space5);
Texture2D<float> srv_5_2: register(t2, space5);
VoxelUpscale CreateVoxelUpscale()
{
	VoxelUpscale result;
	result.srv.tex_downsampled = srv_5_0;
	result.srv.tex_gi_prev = srv_5_1;
	result.srv.tex_depth_prev = srv_5_2;
	return result;
}
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
