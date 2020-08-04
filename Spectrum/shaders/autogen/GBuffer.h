#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBuffer.h"
Texture2D srv_5_0: register(t0, space5);
Texture2D srv_5_1: register(t1, space5);
Texture2D srv_5_2: register(t2, space5);
Texture2D srv_5_3: register(t3, space5);
Texture2D<float2> srv_5_4: register(t4, space5);
GBuffer CreateGBuffer()
{
	GBuffer result;
	result.srv.albedo = srv_5_0;
	result.srv.normals = srv_5_1;
	result.srv.specular = srv_5_2;
	result.srv.depth = srv_5_3;
	result.srv.motion = srv_5_4;
	return result;
}
static const GBuffer gBuffer_global = CreateGBuffer();
const GBuffer GetGBuffer(){ return gBuffer_global; }
