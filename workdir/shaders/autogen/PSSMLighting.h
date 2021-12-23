#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMLighting.h"
Texture2D<float> srv_6_0: register(t0, space6);
Texture2D<float4> srv_6_1: register(t1, space6);
Texture2D<float4> srv_6_2: register(t2, space6);
Texture2D<float4> srv_6_3: register(t3, space6);
Texture2D<float> srv_6_4: register(t4, space6);
Texture2D<float2> srv_6_5: register(t5, space6);
struct Pass_PSSMLighting
{
uint srv_0;
uint srv_1;
uint srv_2;
uint srv_3;
uint srv_4;
uint srv_5;
};
ConstantBuffer<Pass_PSSMLighting> pass_PSSMLighting: register( b2, space6);
const PSSMLighting CreatePSSMLighting()
{
	PSSMLighting result;
	result.srv.light_mask = (pass_PSSMLighting.srv_0 );
	result.srv.gbuffer.albedo = (pass_PSSMLighting.srv_1 );
	result.srv.gbuffer.normals = (pass_PSSMLighting.srv_2 );
	result.srv.gbuffer.specular = (pass_PSSMLighting.srv_3 );
	result.srv.gbuffer.depth = (pass_PSSMLighting.srv_4 );
	result.srv.gbuffer.motion = (pass_PSSMLighting.srv_5 );
	return result;
}
#ifndef NO_GLOBAL
static const PSSMLighting pSSMLighting_global = CreatePSSMLighting();
const PSSMLighting GetPSSMLighting(){ return pSSMLighting_global; }
#endif
