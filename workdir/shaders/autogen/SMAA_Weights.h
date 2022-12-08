#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Weights.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SMAA_Weights: register( b2, space5);
 ConstantBuffer<SMAA_Weights> CreateSMAA_Weights()
{
	return ResourceDescriptorHeap[pass_SMAA_Weights.offset];
}
#ifndef NO_GLOBAL
static const SMAA_Weights sMAA_Weights_global = CreateSMAA_Weights();
const SMAA_Weights GetSMAA_Weights(){ return sMAA_Weights_global; }
#endif
