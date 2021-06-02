#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMConstants.h"
ConstantBuffer<PSSMConstants_cb> cb_4_0:register(b0,space4);
struct Pass_PSSMConstants
{
};
ConstantBuffer<Pass_PSSMConstants> pass_PSSMConstants: register( b2, space4);
const PSSMConstants CreatePSSMConstants()
{
	PSSMConstants result;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMConstants pSSMConstants_global = CreatePSSMConstants();
const PSSMConstants GetPSSMConstants(){ return pSSMConstants_global; }
#endif
