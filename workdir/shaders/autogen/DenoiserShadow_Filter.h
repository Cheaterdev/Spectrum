#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_Filter.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_Filter: register( b2, space4);
 ConstantBuffer<DenoiserShadow_Filter> CreateDenoiserShadow_Filter()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_Filter.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_Filter denoiserShadow_Filter_global = CreateDenoiserShadow_Filter();
const DenoiserShadow_Filter GetDenoiserShadow_Filter(){ return denoiserShadow_Filter_global; }
#endif
