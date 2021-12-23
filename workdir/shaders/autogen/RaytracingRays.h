#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/RaytracingRays.h"
ConstantBuffer<RaytracingRays_cb> cb_6_0:register(b0,space6);
RWTexture2D<float4> uav_6_0: register(u0, space6);
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float4> srv_6_1: register(t1, space6);
Texture2D<float4> srv_6_2: register(t2, space6);
Texture2D<float> srv_6_3: register(t3, space6);
Texture2D<float2> srv_6_4: register(t4, space6);
struct Pass_RaytracingRays
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint uav_0;
};
ConstantBuffer<Pass_RaytracingRays> pass_RaytracingRays: register( b2, space6);
const RaytracingRays CreateRaytracingRays()
{
	RaytracingRays result;
	result.cb = cb_6_0;
	result.uav.output = (pass_RaytracingRays.uav_0 );
	result.srv.gbuffer.albedo = (pass_RaytracingRays.srv_0 );
	result.srv.gbuffer.normals = (pass_RaytracingRays.srv_1 );
	result.srv.gbuffer.specular = (pass_RaytracingRays.srv_2 );
	result.srv.gbuffer.depth = (pass_RaytracingRays.srv_3 );
	result.srv.gbuffer.motion = (pass_RaytracingRays.srv_4 );
	return result;
}
#ifndef NO_GLOBAL
static const RaytracingRays raytracingRays_global = CreateRaytracingRays();
const RaytracingRays GetRaytracingRays(){ return raytracingRays_global; }
#endif
