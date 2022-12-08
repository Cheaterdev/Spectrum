#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FSR.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FSR: register( b2, space4);
 ConstantBuffer<FSR> CreateFSR()
{
	return ResourceDescriptorHeap[pass_FSR.offset];
}
#ifndef NO_GLOBAL
static const FSR fSR_global = CreateFSR();
const FSR GetFSR(){ return fSR_global; }
#endif
