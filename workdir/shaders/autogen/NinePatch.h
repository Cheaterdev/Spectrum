#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/NinePatch.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_NinePatch: register( b2, space4);
 ConstantBuffer<NinePatch> CreateNinePatch()
{
	return ResourceDescriptorHeap[pass_NinePatch.offset];
}
#ifndef NO_GLOBAL
static const NinePatch ninePatch_global = CreateNinePatch();
const NinePatch GetNinePatch(){ return ninePatch_global; }
#endif
