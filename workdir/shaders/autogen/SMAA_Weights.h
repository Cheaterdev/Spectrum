#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Weights.h"
ConstantBuffer<SMAA_Weights> pass_SMAA_Weights: register( b2, space5);
const SMAA_Weights CreateSMAA_Weights()
{
	return pass_SMAA_Weights;
}
#ifndef NO_GLOBAL
static const SMAA_Weights sMAA_Weights_global = CreateSMAA_Weights();
const SMAA_Weights GetSMAA_Weights(){ return sMAA_Weights_global; }
#endif
