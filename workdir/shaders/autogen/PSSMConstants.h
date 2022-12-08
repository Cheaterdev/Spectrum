#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMConstants.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_PSSMConstants: register( b2, space4);
 ConstantBuffer<PSSMConstants> CreatePSSMConstants()
{
	return ResourceDescriptorHeap[pass_PSSMConstants.offset];
}
#ifndef NO_GLOBAL
static const PSSMConstants pSSMConstants_global = CreatePSSMConstants();
const PSSMConstants GetPSSMConstants(){ return pSSMConstants_global; }
#endif
