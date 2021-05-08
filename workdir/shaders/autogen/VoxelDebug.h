#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelDebug.h"
Texture3D<float4> srv_4_0: register(t0, space4);
Texture2D<float4> srv_4_1: register(t1, space4);
Texture2D<float4> srv_4_2: register(t2, space4);
Texture2D<float4> srv_4_3: register(t3, space4);
Texture2D<float> srv_4_4: register(t4, space4);
Texture2D<float2> srv_4_5: register(t5, space4);
struct Pass_VoxelDebug
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint srv_5;
};
ConstantBuffer<Pass_VoxelDebug> pass_VoxelDebug: register( b2, space4);
const VoxelDebug CreateVoxelDebug()
{
	VoxelDebug result;
	Pass_VoxelDebug pass;
	result.srv.volume = srv_4_0;
	result.srv.gbuffer.albedo = srv_4_1;
	result.srv.gbuffer.normals = srv_4_2;
	result.srv.gbuffer.specular = srv_4_3;
	result.srv.gbuffer.depth = srv_4_4;
	result.srv.gbuffer.motion = srv_4_5;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelDebug voxelDebug_global = CreateVoxelDebug();
const VoxelDebug GetVoxelDebug(){ return voxelDebug_global; }
#endif
