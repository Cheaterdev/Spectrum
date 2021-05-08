#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelScreen.h"
Texture3D<float4> srv_4_0: register(t0, space4);
TextureCube<float4> srv_4_1: register(t1, space4);
Texture2D<float4> srv_4_2: register(t2, space4);
Texture2D<float> srv_4_3: register(t3, space4);
Texture2D<float> srv_4_4: register(t4, space4);
Texture2D<float4> srv_4_5: register(t5, space4);
Texture2D<float4> srv_4_6: register(t6, space4);
Texture2D<float4> srv_4_7: register(t7, space4);
Texture2D<float> srv_4_8: register(t8, space4);
Texture2D<float2> srv_4_9: register(t9, space4);
struct Pass_VoxelScreen
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint srv_5;
uint srv_6;
uint srv_7;
uint srv_8;
uint srv_9;
};
ConstantBuffer<Pass_VoxelScreen> pass_VoxelScreen: register( b2, space4);
const VoxelScreen CreateVoxelScreen()
{
	VoxelScreen result;
	Pass_VoxelScreen pass;
	result.srv.voxels = srv_4_0;
	result.srv.tex_cube = srv_4_1;
	result.srv.prev_gi = srv_4_2;
	result.srv.prev_frames = srv_4_3;
	result.srv.prev_depth = srv_4_4;
	result.srv.gbuffer.albedo = srv_4_5;
	result.srv.gbuffer.normals = srv_4_6;
	result.srv.gbuffer.specular = srv_4_7;
	result.srv.gbuffer.depth = srv_4_8;
	result.srv.gbuffer.motion = srv_4_9;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelScreen voxelScreen_global = CreateVoxelScreen();
const VoxelScreen GetVoxelScreen(){ return voxelScreen_global; }
#endif
