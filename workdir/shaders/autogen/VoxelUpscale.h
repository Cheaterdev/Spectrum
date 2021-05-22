#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelUpscale.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float4> srv_6_1: register(t1, space6);
Texture2D<float> srv_6_2: register(t2, space6);
struct Pass_VoxelUpscale
{
uint srv_0;
uint srv_1;
uint srv_2;
};
ConstantBuffer<Pass_VoxelUpscale> pass_VoxelUpscale: register( b2, space6);
const VoxelUpscale CreateVoxelUpscale()
{
	VoxelUpscale result;
	Pass_VoxelUpscale pass;
	result.srv.tex_downsampled = srv_6_0;
	result.srv.tex_gi_prev = srv_6_1;
	result.srv.tex_depth_prev = srv_6_2;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelUpscale voxelUpscale_global = CreateVoxelUpscale();
const VoxelUpscale GetVoxelUpscale(){ return voxelUpscale_global; }
#endif
