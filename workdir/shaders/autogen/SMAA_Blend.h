#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Blend.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SMAA_Blend: register( b2, space5);
 ConstantBuffer<SMAA_Blend> CreateSMAA_Blend()
{
	return ResourceDescriptorHeap[pass_SMAA_Blend.offset];
}
#ifndef NO_GLOBAL
static const SMAA_Blend sMAA_Blend_global = CreateSMAA_Blend();
const SMAA_Blend GetSMAA_Blend(){ return sMAA_Blend_global; }
#endif
