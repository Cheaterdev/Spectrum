#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBufferQuality.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GBufferQuality: register( b2, space6);
 ConstantBuffer<GBufferQuality> CreateGBufferQuality()
{
	return ResourceDescriptorHeap[pass_GBufferQuality.offset];
}
#ifndef NO_GLOBAL
static const GBufferQuality gBufferQuality_global = CreateGBufferQuality();
const GBufferQuality GetGBufferQuality(){ return gBufferQuality_global; }
#endif
