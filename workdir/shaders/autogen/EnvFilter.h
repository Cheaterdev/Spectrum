#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvFilter.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_EnvFilter: register( b2, space5);
 ConstantBuffer<EnvFilter> CreateEnvFilter()
{
	return ResourceDescriptorHeap[pass_EnvFilter.offset];
}
#ifndef NO_GLOBAL
static const EnvFilter envFilter_global = CreateEnvFilter();
const EnvFilter GetEnvFilter(){ return envFilter_global; }
#endif
