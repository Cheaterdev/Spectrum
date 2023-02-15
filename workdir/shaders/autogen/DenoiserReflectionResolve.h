#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserReflectionResolve.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserReflectionResolve: register( b2, space5);
 ConstantBuffer<DenoiserReflectionResolve> CreateDenoiserReflectionResolve()
{
	return ResourceDescriptorHeap[pass_DenoiserReflectionResolve.offset];
}
#ifndef NO_GLOBAL
static const DenoiserReflectionResolve denoiserReflectionResolve_global = CreateDenoiserReflectionResolve();
const DenoiserReflectionResolve GetDenoiserReflectionResolve(){ return denoiserReflectionResolve_global; }
#endif
