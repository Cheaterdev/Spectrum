#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferDownsample.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GBufferDownsample: register( b2, space6);
 ConstantBuffer<GBufferDownsample> CreateGBufferDownsample()
{
	return ResourceDescriptorHeap[pass_GBufferDownsample.offset];
}
#ifndef NO_GLOBAL
static const GBufferDownsample gBufferDownsample_global = CreateGBufferDownsample();
const GBufferDownsample GetGBufferDownsample(){ return gBufferDownsample_global; }
#endif
