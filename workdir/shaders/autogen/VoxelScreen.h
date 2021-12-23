#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelScreen.h"
Texture3D<float4> srv_5_0: register(t0, space5);
TextureCube<float4> srv_5_1: register(t1, space5);
Texture2D<float4> srv_5_2: register(t2, space5);
Texture2D<float> srv_5_3: register(t3, space5);
Texture2D<float> srv_5_4: register(t4, space5);
Texture2D<float4> srv_5_5: register(t5, space5);
Texture2D<float4> srv_5_6: register(t6, space5);
Texture2D<float4> srv_5_7: register(t7, space5);
Texture2D<float> srv_5_8: register(t8, space5);
Texture2D<float2> srv_5_9: register(t9, space5);
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
ConstantBuffer<Pass_VoxelScreen> pass_VoxelScreen: register( b2, space5);
const VoxelScreen CreateVoxelScreen()
{
	VoxelScreen result;
	result.srv.voxels = (pass_VoxelScreen.srv_0 );
	result.srv.tex_cube = (pass_VoxelScreen.srv_1 );
	result.srv.prev_gi = (pass_VoxelScreen.srv_2 );
	result.srv.prev_frames = (pass_VoxelScreen.srv_3 );
	result.srv.prev_depth = (pass_VoxelScreen.srv_4 );
	result.srv.gbuffer.albedo = (pass_VoxelScreen.srv_5 );
	result.srv.gbuffer.normals = (pass_VoxelScreen.srv_6 );
	result.srv.gbuffer.specular = (pass_VoxelScreen.srv_7 );
	result.srv.gbuffer.depth = (pass_VoxelScreen.srv_8 );
	result.srv.gbuffer.motion = (pass_VoxelScreen.srv_9 );
	return result;
}
#ifndef NO_GLOBAL
static const VoxelScreen voxelScreen_global = CreateVoxelScreen();
const VoxelScreen GetVoxelScreen(){ return voxelScreen_global; }
#endif
