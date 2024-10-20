#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif

#include "layout/DefaultLayout.h"
#include "tables/DispatchParameters.h"

#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif

ConstantBuffer< CB > pass_DispatchParameters: register( b2, space6);

ConstantBuffer<DispatchParameters> CreateDispatchParameters()
{
	return ResourceDescriptorHeap[pass_DispatchParameters.offset];
}
			
#ifndef NO_GLOBAL
static const DispatchParameters dispatchParameters_global = CreateDispatchParameters();
const DispatchParameters GetDispatchParameters(){ return dispatchParameters_global; }
#endif