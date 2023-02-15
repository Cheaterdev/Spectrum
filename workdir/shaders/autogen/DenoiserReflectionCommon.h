#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserReflectionCommon.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserReflectionCommon: register( b2, space4);
 ConstantBuffer<DenoiserReflectionCommon> CreateDenoiserReflectionCommon()
{
	return ResourceDescriptorHeap[pass_DenoiserReflectionCommon.offset];
}
#ifndef NO_GLOBAL
static const DenoiserReflectionCommon denoiserReflectionCommon_global = CreateDenoiserReflectionCommon();
const DenoiserReflectionCommon GetDenoiserReflectionCommon(){ return denoiserReflectionCommon_global; }
#endif
