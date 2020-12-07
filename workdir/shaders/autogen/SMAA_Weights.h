#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Weights.h"
Texture2D<float4> srv_4_0: register(t0, space4);
Texture2D<float4> srv_4_1: register(t1, space4);
Texture2D<float4> srv_4_2: register(t2, space4);
SMAA_Weights CreateSMAA_Weights()
{
	SMAA_Weights result;
	result.srv.areaTex = srv_4_0;
	result.srv.searchTex = srv_4_1;
	result.srv.edgesTex = srv_4_2;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Weights sMAA_Weights_global = CreateSMAA_Weights();
const SMAA_Weights GetSMAA_Weights(){ return sMAA_Weights_global; }
#endif
