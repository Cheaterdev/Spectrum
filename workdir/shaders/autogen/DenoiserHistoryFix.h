#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserHistoryFix.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserHistoryFix: register( b2, space6);
 ConstantBuffer<DenoiserHistoryFix> CreateDenoiserHistoryFix()
{
	return ResourceDescriptorHeap[pass_DenoiserHistoryFix.offset];
}
#ifndef NO_GLOBAL
static const DenoiserHistoryFix denoiserHistoryFix_global = CreateDenoiserHistoryFix();
const DenoiserHistoryFix GetDenoiserHistoryFix(){ return denoiserHistoryFix_global; }
#endif
