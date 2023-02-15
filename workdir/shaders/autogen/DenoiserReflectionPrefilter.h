#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserReflectionPrefilter.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserReflectionPrefilter: register( b2, space5);
 ConstantBuffer<DenoiserReflectionPrefilter> CreateDenoiserReflectionPrefilter()
{
	return ResourceDescriptorHeap[pass_DenoiserReflectionPrefilter.offset];
}
#ifndef NO_GLOBAL
static const DenoiserReflectionPrefilter denoiserReflectionPrefilter_global = CreateDenoiserReflectionPrefilter();
const DenoiserReflectionPrefilter GetDenoiserReflectionPrefilter(){ return denoiserReflectionPrefilter_global; }
#endif
