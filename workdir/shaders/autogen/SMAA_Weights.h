#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Weights.h"
Texture2D srv_3_0: register(t0, space3);
Texture2D srv_3_1: register(t1, space3);
Texture2D srv_3_2: register(t2, space3);
SMAA_Weights CreateSMAA_Weights()
{
	SMAA_Weights result;
	result.srv.areaTex = srv_3_0;
	result.srv.searchTex = srv_3_1;
	result.srv.edgesTex = srv_3_2;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Weights sMAA_Weights_global = CreateSMAA_Weights();
const SMAA_Weights GetSMAA_Weights(){ return sMAA_Weights_global; }
#endif
