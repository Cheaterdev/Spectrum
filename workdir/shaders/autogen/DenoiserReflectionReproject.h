#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserReflectionReproject.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DenoiserReflectionReproject: register( b2, space5);
 ConstantBuffer<DenoiserReflectionReproject> CreateDenoiserReflectionReproject()
{
	return ResourceDescriptorHeap[pass_DenoiserReflectionReproject.offset];
}
#ifndef NO_GLOBAL
static const DenoiserReflectionReproject denoiserReflectionReproject_global = CreateDenoiserReflectionReproject();
const DenoiserReflectionReproject GetDenoiserReflectionReproject(){ return denoiserReflectionReproject_global; }
#endif
