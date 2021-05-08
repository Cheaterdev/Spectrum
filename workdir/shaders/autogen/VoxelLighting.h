#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelLighting.h"
ConstantBuffer<VoxelLighting_cb> cb_4_0:register(b0,space4);
Texture3D<float4> srv_4_0: register(t0, space4);
Texture3D<float4> srv_4_1: register(t1, space4);
Texture3D<float4> srv_4_2: register(t2, space4);
TextureCube<float4> srv_4_3: register(t3, space4);
RWTexture3D<float4> uav_4_0: register(u0, space4);
Texture2D<float> srv_4_4: register(t4, space4);
StructuredBuffer<Camera> srv_4_5: register(t5, space4);
StructuredBuffer<int3> srv_4_6: register(t6, space4);
struct Pass_VoxelLighting
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint srv_5;
uint srv_6;
uint uav_0;
};
ConstantBuffer<Pass_VoxelLighting> pass_VoxelLighting: register( b2, space4);
const VoxelLighting CreateVoxelLighting()
{
	VoxelLighting result;
	Pass_VoxelLighting pass;
	result.cb = cb_4_0;
	result.srv.albedo = srv_4_0;
	result.srv.normals = srv_4_1;
	result.srv.lower = srv_4_2;
	result.srv.tex_cube = srv_4_3;
	result.uav.output = uav_4_0;
	result.srv.pssmGlobal.light_buffer = srv_4_4;
	result.srv.pssmGlobal.light_camera = srv_4_5;
	result.srv.params.tiles = srv_4_6;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelLighting voxelLighting_global = CreateVoxelLighting();
const VoxelLighting GetVoxelLighting(){ return voxelLighting_global; }
#endif
