#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/InitDispatch.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_InitDispatch: register( b2, space5);
 ConstantBuffer<InitDispatch> CreateInitDispatch()
{
	return ResourceDescriptorHeap[pass_InitDispatch.offset];
}
#ifndef NO_GLOBAL
static const InitDispatch initDispatch_global = CreateInitDispatch();
const InitDispatch GetInitDispatch(){ return initDispatch_global; }
#endif
