#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/VoxelLighting.h"
ConstantBuffer<VoxelLighting_cb> cb_3_0:register(b0,space3);
Texture3D<float4> srv_3_0: register(t0, space3);
Texture3D<float4> srv_3_1: register(t1, space3);
Texture3D<float4> srv_3_2: register(t2, space3);
TextureCube<float4> srv_3_3: register(t3, space3);
StructuredBuffer<int3> srv_3_4: register(t4, space3);
RWTexture3D<float4> uav_3_0: register(u0, space3);
Texture2D<float> srv_3_5: register(t5, space3);
StructuredBuffer<Camera> srv_3_6: register(t6, space3);
VoxelLighting CreateVoxelLighting()
{
	VoxelLighting result;
	result.cb = cb_3_0;
	result.srv.albedo = srv_3_0;
	result.srv.normals = srv_3_1;
	result.srv.lower = srv_3_2;
	result.srv.tex_cube = srv_3_3;
	result.srv.visibility = srv_3_4;
	result.uav.output = uav_3_0;
	result.srv.pssmGlobal.light_buffer = srv_3_5;
	result.srv.pssmGlobal.light_camera = srv_3_6;
	return result;
}
#ifndef NO_GLOBAL
static const VoxelLighting voxelLighting_global = CreateVoxelLighting();
const VoxelLighting GetVoxelLighting(){ return voxelLighting_global; }
#endif
