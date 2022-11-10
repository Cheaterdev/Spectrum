#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserHistoryFix.h"
ConstantBuffer<DenoiserHistoryFix> pass_DenoiserHistoryFix: register( b2, space6);
const DenoiserHistoryFix CreateDenoiserHistoryFix()
{
	return pass_DenoiserHistoryFix;
}
#ifndef NO_GLOBAL
static const DenoiserHistoryFix denoiserHistoryFix_global = CreateDenoiserHistoryFix();
const DenoiserHistoryFix GetDenoiserHistoryFix(){ return denoiserHistoryFix_global; }
#endif
