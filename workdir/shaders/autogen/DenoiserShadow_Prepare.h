#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_Prepare.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_Prepare: register( b2, space4);
 ConstantBuffer<DenoiserShadow_Prepare> CreateDenoiserShadow_Prepare()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_Prepare.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_Prepare denoiserShadow_Prepare_global = CreateDenoiserShadow_Prepare();
const DenoiserShadow_Prepare GetDenoiserShadow_Prepare(){ return denoiserShadow_Prepare_global; }
#endif
