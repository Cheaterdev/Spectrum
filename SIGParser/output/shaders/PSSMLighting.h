#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMLighting.h"
Texture2D<float> srv_4_0: register(t0, space4);
Texture3D<float4> srv_4_1: register(t1, space4);
Texture2D srv_4_2: register(t2, space4);
Texture2D srv_4_3: register(t3, space4);
Texture2D srv_4_4: register(t4, space4);
Texture2D srv_4_5: register(t5, space4);
PSSMLighting CreatePSSMLighting()
{
	PSSMLighting result;
	result.srv.light_mask = srv_4_0;
	result.srv.brdf = srv_4_1;
	result.srv.gbuffer.albedo = srv_4_2;
	result.srv.gbuffer.normals = srv_4_3;
	result.srv.gbuffer.specular = srv_4_4;
	result.srv.gbuffer.depth = srv_4_5;
	result.gbuffer = CreateGBuffer(result.srv.gbuffer);
	return result;
}
static const PSSMLighting pSSMLighting_global = CreatePSSMLighting();
const PSSMLighting GetPSSMLighting(){ return pSSMLighting_global; }
