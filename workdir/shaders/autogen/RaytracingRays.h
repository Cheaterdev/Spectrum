#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/RaytracingRays.h"
ConstantBuffer<RaytracingRays_cb> cb_5_0:register(b0,space5);
RWTexture2D<float4> uav_5_0: register(u0, space5);
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float4> srv_5_1: register(t1, space5);
Texture2D<float4> srv_5_2: register(t2, space5);
Texture2D<float> srv_5_3: register(t3, space5);
Texture2D<float2> srv_5_4: register(t4, space5);
RaytracingRays CreateRaytracingRays()
{
	RaytracingRays result;
	result.cb = cb_5_0;
	result.uav.output = uav_5_0;
	result.srv.gbuffer.albedo = srv_5_0;
	result.srv.gbuffer.normals = srv_5_1;
	result.srv.gbuffer.specular = srv_5_2;
	result.srv.gbuffer.depth = srv_5_3;
	result.srv.gbuffer.motion = srv_5_4;
	return result;
}
#ifndef NO_GLOBAL
static const RaytracingRays raytracingRays_global = CreateRaytracingRays();
const RaytracingRays GetRaytracingRays(){ return raytracingRays_global; }
#endif
