#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_FilterLast.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_FilterLast: register( b2, space5);
 ConstantBuffer<DenoiserShadow_FilterLast> CreateDenoiserShadow_FilterLast()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_FilterLast.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_FilterLast denoiserShadow_FilterLast_global = CreateDenoiserShadow_FilterLast();
const DenoiserShadow_FilterLast GetDenoiserShadow_FilterLast(){ return denoiserShadow_FilterLast_global; }
#endif
