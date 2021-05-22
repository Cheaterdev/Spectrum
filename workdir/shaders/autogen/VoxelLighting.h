#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelLighting.h"
ConstantBuffer<VoxelLighting_cb> cb_5_0:register(b0,space5);
Texture3D<float4> srv_5_0: register(t0, space5);
Texture3D<float4> srv_5_1: register(t1, space5);
Texture3D<float4> srv_5_2: register(t2, space5);
TextureCube<float4> srv_5_3: register(t3, space5);
RWTexture3D<float4> uav_5_0: register(u0, space5);
Texture2D<float> srv_5_4: register(t4, space5);
StructuredBuffer<Camera> srv_5_5: register(t5, space5);
StructuredBuffer<int3> srv_5_6: register(t6, space5);
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
ConstantBuffer<Pass_VoxelLighting> pass_VoxelLighting: register( b2, space5);
const VoxelLighting CreateVoxelLighting()
{
	VoxelLighting result;
	Pass_VoxelLighting pass;
	result.cb = cb_5_0;
	result.srv.albedo = srv_5_0;
	result.srv.normals = srv_5_1;
	result.srv.lower = srv_5_2;
	result.srv.tex_cube = srv_5_3;
	result.uav.output = uav_5_0;
	result.srv.pssmGlobal.light_buffer = srv_5_4;
	result.srv.pssmGlobal.light_camera = srv_5_5;
	result.srv.params.tiles = srv_5_6;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelLighting voxelLighting_global = CreateVoxelLighting();
const VoxelLighting GetVoxelLighting(){ return voxelLighting_global; }
#endif
