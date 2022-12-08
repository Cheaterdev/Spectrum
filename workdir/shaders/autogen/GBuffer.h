#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBuffer.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_GBuffer: register( b2, space6);
 ConstantBuffer<GBuffer> CreateGBuffer()
{
	return ResourceDescriptorHeap[pass_GBuffer.offset];
}
#ifndef NO_GLOBAL
static const GBuffer gBuffer_global = CreateGBuffer();
const GBuffer GetGBuffer(){ return gBuffer_global; }
#endif
