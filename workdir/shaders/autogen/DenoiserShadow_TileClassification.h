#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_TileClassification.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_TileClassification: register( b2, space4);
 ConstantBuffer<DenoiserShadow_TileClassification> CreateDenoiserShadow_TileClassification()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_TileClassification.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_TileClassification denoiserShadow_TileClassification_global = CreateDenoiserShadow_TileClassification();
const DenoiserShadow_TileClassification GetDenoiserShadow_TileClassification(){ return denoiserShadow_TileClassification_global; }
#endif
