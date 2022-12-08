#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMData.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_PSSMData: register( b2, space5);
 ConstantBuffer<PSSMData> CreatePSSMData()
{
	return ResourceDescriptorHeap[pass_PSSMData.offset];
}
#ifndef NO_GLOBAL
static const PSSMData pSSMData_global = CreatePSSMData();
const PSSMData GetPSSMData(){ return pSSMData_global; }
#endif
