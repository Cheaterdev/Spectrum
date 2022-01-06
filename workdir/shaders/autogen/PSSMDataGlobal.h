#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
ConstantBuffer<PSSMDataGlobal> pass_PSSMDataGlobal: register( b2, space5);
const PSSMDataGlobal CreatePSSMDataGlobal()
{
	return pass_PSSMDataGlobal;
}
#ifndef NO_GLOBAL
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
#endif
