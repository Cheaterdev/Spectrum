#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMConstants.h"
ConstantBuffer<PSSMConstants> pass_PSSMConstants: register( b2, space4);
const PSSMConstants CreatePSSMConstants()
{
	return pass_PSSMConstants;
}
#ifndef NO_GLOBAL
static const PSSMConstants pSSMConstants_global = CreatePSSMConstants();
const PSSMConstants GetPSSMConstants(){ return pSSMConstants_global; }
#endif
