#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/RaytracingRays.h"
RWTexture2D<float4> uav_4_0: register(u0, space4);
Texture2D srv_4_0: register(t0, space4);
Texture2D srv_4_1: register(t1, space4);
Texture2D srv_4_2: register(t2, space4);
Texture2D srv_4_3: register(t3, space4);
Texture2D<float2> srv_4_4: register(t4, space4);
RaytracingRays CreateRaytracingRays()
{
	RaytracingRays result;
	result.uav.output = uav_4_0;
	result.srv.gbuffer.albedo = srv_4_0;
	result.srv.gbuffer.normals = srv_4_1;
	result.srv.gbuffer.specular = srv_4_2;
	result.srv.gbuffer.depth = srv_4_3;
	result.srv.gbuffer.motion = srv_4_4;
	return result;
}
#ifndef NO_GLOBAL
static const RaytracingRays raytracingRays_global = CreateRaytracingRays();
const RaytracingRays GetRaytracingRays(){ return raytracingRays_global; }
#endif
