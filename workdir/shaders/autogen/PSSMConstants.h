#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMConstants.h"
ConstantBuffer<PSSMConstants_cb> cb_3_0:register(b0,space3);
PSSMConstants CreatePSSMConstants()
{
	PSSMConstants result;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMConstants pSSMConstants_global = CreatePSSMConstants();
const PSSMConstants GetPSSMConstants(){ return pSSMConstants_global; }
#endif
