#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMData.h"
ConstantBuffer<PSSMData> pass_PSSMData: register( b2, space5);
const PSSMData CreatePSSMData()
{
	return pass_PSSMData;
}
#ifndef NO_GLOBAL
static const PSSMData pSSMData_global = CreatePSSMData();
const PSSMData GetPSSMData(){ return pSSMData_global; }
#endif
