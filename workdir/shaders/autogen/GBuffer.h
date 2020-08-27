#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBuffer.h"
Texture2D srv_4_0: register(t0, space4);
Texture2D srv_4_1: register(t1, space4);
Texture2D srv_4_2: register(t2, space4);
Texture2D srv_4_3: register(t3, space4);
Texture2D<float2> srv_4_4: register(t4, space4);
GBuffer CreateGBuffer()
{
	GBuffer result;
	result.srv.albedo = srv_4_0;
	result.srv.normals = srv_4_1;
	result.srv.specular = srv_4_2;
	result.srv.depth = srv_4_3;
	result.srv.motion = srv_4_4;
	return result;
}
#ifndef NO_GLOBAL
static const GBuffer gBuffer_global = CreateGBuffer();
const GBuffer GetGBuffer(){ return gBuffer_global; }
#endif
