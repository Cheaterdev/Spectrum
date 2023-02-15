#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ReflectionCombine.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_ReflectionCombine: register( b2, space4);
 ConstantBuffer<ReflectionCombine> CreateReflectionCombine()
{
	return ResourceDescriptorHeap[pass_ReflectionCombine.offset];
}
#ifndef NO_GLOBAL
static const ReflectionCombine reflectionCombine_global = CreateReflectionCombine();
const ReflectionCombine GetReflectionCombine(){ return reflectionCombine_global; }
#endif
