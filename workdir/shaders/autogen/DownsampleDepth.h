#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DownsampleDepth.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DownsampleDepth: register( b2, space4);
 ConstantBuffer<DownsampleDepth> CreateDownsampleDepth()
{
	return ResourceDescriptorHeap[pass_DownsampleDepth.offset];
}
#ifndef NO_GLOBAL
static const DownsampleDepth downsampleDepth_global = CreateDownsampleDepth();
const DownsampleDepth GetDownsampleDepth(){ return downsampleDepth_global; }
#endif
