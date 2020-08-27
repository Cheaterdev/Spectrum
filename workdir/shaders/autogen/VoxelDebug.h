#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelDebug.h"
Texture3D<float4> srv_3_0: register(t0, space3);
Texture2D srv_3_1: register(t1, space3);
Texture2D srv_3_2: register(t2, space3);
Texture2D srv_3_3: register(t3, space3);
Texture2D srv_3_4: register(t4, space3);
Texture2D<float2> srv_3_5: register(t5, space3);
VoxelDebug CreateVoxelDebug()
{
	VoxelDebug result;
	result.srv.volume = srv_3_0;
	result.srv.gbuffer.albedo = srv_3_1;
	result.srv.gbuffer.normals = srv_3_2;
	result.srv.gbuffer.specular = srv_3_3;
	result.srv.gbuffer.depth = srv_3_4;
	result.srv.gbuffer.motion = srv_3_5;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelDebug voxelDebug_global = CreateVoxelDebug();
const VoxelDebug GetVoxelDebug(){ return voxelDebug_global; }
#endif
