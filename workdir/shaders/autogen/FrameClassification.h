#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassification.h"
ConstantBuffer<FrameClassification> pass_FrameClassification: register( b2, space6);
const FrameClassification CreateFrameClassification()
{
	return pass_FrameClassification;
}
#ifndef NO_GLOBAL
static const FrameClassification frameClassification_global = CreateFrameClassification();
const FrameClassification GetFrameClassification(){ return frameClassification_global; }
#endif
