#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMLighting.h"
Texture2D<float> srv_5_0: register(t0, space5);
Texture3D<float4> srv_5_1: register(t1, space5);
Texture2D srv_5_2: register(t2, space5);
Texture2D srv_5_3: register(t3, space5);
Texture2D srv_5_4: register(t4, space5);
Texture2D srv_5_5: register(t5, space5);
PSSMLighting CreatePSSMLighting()
{
	PSSMLighting result;
	result.srv.light_mask = srv_5_0;
	result.srv.brdf = srv_5_1;
	result.srv.gbuffer.albedo = srv_5_2;
	result.srv.gbuffer.normals = srv_5_3;
	result.srv.gbuffer.specular = srv_5_4;
	result.srv.gbuffer.depth = srv_5_5;
	result.gbuffer = CreateGBuffer(result.srv.gbuffer);
	return result;
}
static const PSSMLighting pSSMLighting_global = CreatePSSMLighting();
const PSSMLighting GetPSSMLighting(){ return pSSMLighting_global; }
