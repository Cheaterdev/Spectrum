#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/TilingPostprocess.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_TilingPostprocess: register( b2, space2);
 ConstantBuffer<TilingPostprocess> CreateTilingPostprocess()
{
	return ResourceDescriptorHeap[pass_TilingPostprocess.offset];
}
#ifndef NO_GLOBAL
static const TilingPostprocess tilingPostprocess_global = CreateTilingPostprocess();
const TilingPostprocess GetTilingPostprocess(){ return tilingPostprocess_global; }
#endif
