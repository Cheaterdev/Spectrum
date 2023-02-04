#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_Fileter.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserShadow_Fileter: register( b2, space4);
 ConstantBuffer<DenoiserShadow_Fileter> CreateDenoiserShadow_Fileter()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_Fileter.offset];
}
#ifndef NO_GLOBAL
static const DenoiserShadow_Fileter denoiserShadow_Fileter_global = CreateDenoiserShadow_Fileter();
const DenoiserShadow_Fileter GetDenoiserShadow_Fileter(){ return denoiserShadow_Fileter_global; }
#endif
