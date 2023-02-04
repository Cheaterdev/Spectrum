#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_FilterLocal.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_FilterLocal: register( b2, space5);
 ConstantBuffer<DenoiserShadow_FilterLocal> CreateDenoiserShadow_FilterLocal()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_FilterLocal.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_FilterLocal denoiserShadow_FilterLocal_global = CreateDenoiserShadow_FilterLocal();
const DenoiserShadow_FilterLocal GetDenoiserShadow_FilterLocal(){ return denoiserShadow_FilterLocal_global; }
#endif
