#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassificationInitDispatch.h"
ConstantBuffer<FrameClassificationInitDispatch> pass_FrameClassificationInitDispatch: register( b2, space6);
const FrameClassificationInitDispatch CreateFrameClassificationInitDispatch()
{
	return pass_FrameClassificationInitDispatch;
}
#ifndef NO_GLOBAL
static const FrameClassificationInitDispatch frameClassificationInitDispatch_global = CreateFrameClassificationInitDispatch();
const FrameClassificationInitDispatch GetFrameClassificationInitDispatch(){ return frameClassificationInitDispatch_global; }
#endif
