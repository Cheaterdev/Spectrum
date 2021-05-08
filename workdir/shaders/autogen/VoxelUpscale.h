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
struct Pass_VoxelUpscale
{
uint srv_0;
uint srv_1;
uint srv_2;
};
ConstantBuffer<Pass_VoxelUpscale> pass_VoxelUpscale: register( b2, space5);
const VoxelUpscale CreateVoxelUpscale()
{
	VoxelUpscale result;
	Pass_VoxelUpscale pass;
	result.srv.tex_downsampled = srv_5_0;
	result.srv.tex_gi_prev = srv_5_1;
	result.srv.tex_depth_prev = srv_5_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
#endif
