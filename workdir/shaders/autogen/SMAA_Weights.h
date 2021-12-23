#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Weights.h"
Texture2D<float4> srv_5_0: register(t0, space5);
Texture2D<float4> srv_5_1: register(t1, space5);
Texture2D<float4> srv_5_2: register(t2, space5);
struct Pass_SMAA_Weights
{
uint srv_0;
uint srv_1;
uint srv_2;
};
ConstantBuffer<Pass_SMAA_Weights> pass_SMAA_Weights: register( b2, space5);
const SMAA_Weights CreateSMAA_Weights()
{
	SMAA_Weights result;
	result.srv.areaTex = (pass_SMAA_Weights.srv_0 );
	result.srv.searchTex = (pass_SMAA_Weights.srv_1 );
	result.srv.edgesTex = (pass_SMAA_Weights.srv_2 );
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Weights sMAA_Weights_global = CreateSMAA_Weights();
const SMAA_Weights GetSMAA_Weights(){ return sMAA_Weights_global; }
#endif
