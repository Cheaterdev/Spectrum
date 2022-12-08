#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassification.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FrameClassification: register( b2, space6);
 ConstantBuffer<FrameClassification> CreateFrameClassification()
{
	return ResourceDescriptorHeap[pass_FrameClassification.offset];
}
#ifndef NO_GLOBAL
static const FrameClassification frameClassification_global = CreateFrameClassification();
const FrameClassification GetFrameClassification(){ return frameClassification_global; }
#endif
