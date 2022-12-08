#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassificationInitDispatch.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FrameClassificationInitDispatch: register( b2, space6);
 ConstantBuffer<FrameClassificationInitDispatch> CreateFrameClassificationInitDispatch()
{
	return ResourceDescriptorHeap[pass_FrameClassificationInitDispatch.offset];
}
#ifndef NO_GLOBAL
static const FrameClassificationInitDispatch frameClassificationInitDispatch_global = CreateFrameClassificationInitDispatch();
const FrameClassificationInitDispatch GetFrameClassificationInitDispatch(){ return frameClassificationInitDispatch_global; }
#endif
