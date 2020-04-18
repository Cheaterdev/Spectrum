#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMConstants.h"
ConstantBuffer<PSSMConstants_cb> cb_2_0:register(b0,space2);
PSSMConstants CreatePSSMConstants()
{
	PSSMConstants result;
	result.cb = cb_2_0;
	return result;
}
static const PSSMConstants pSSMConstants_global = CreatePSSMConstants();
const PSSMConstants GetPSSMConstants(){ return pSSMConstants_global; }
