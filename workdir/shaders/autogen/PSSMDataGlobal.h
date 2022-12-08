#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_PSSMDataGlobal: register( b2, space5);
 ConstantBuffer<PSSMDataGlobal> CreatePSSMDataGlobal()
{
	return ResourceDescriptorHeap[pass_PSSMDataGlobal.offset];
}
#ifndef NO_GLOBAL
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
#endif
