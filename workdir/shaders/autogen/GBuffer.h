#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBuffer.h"
Texture2D<float4> srv_6_0: register(t0, space6);
Texture2D<float4> srv_6_1: register(t1, space6);
Texture2D<float4> srv_6_2: register(t2, space6);
Texture2D<float> srv_6_3: register(t3, space6);
Texture2D<float2> srv_6_4: register(t4, space6);
struct Pass_GBuffer
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
};
ConstantBuffer<Pass_GBuffer> pass_GBuffer: register( b2, space6);
const GBuffer CreateGBuffer()
{
	GBuffer result;
	result.srv.albedo = srv_6_0;
	result.srv.normals = srv_6_1;
	result.srv.specular = srv_6_2;
	result.srv.depth = srv_6_3;
	result.srv.motion = srv_6_4;
	return result;
}
#ifndef NO_GLOBAL
static const GBuffer gBuffer_global = CreateGBuffer();
const GBuffer GetGBuffer(){ return gBuffer_global; }
#endif
