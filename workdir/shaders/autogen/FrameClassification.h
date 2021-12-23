#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassification.h"
Texture2D<float> srv_6_0: register(t0, space6);
AppendStructuredBuffer<uint2> uav_6_0: register(u0, space6);
AppendStructuredBuffer<uint2> uav_6_1: register(u1, space6);
struct Pass_FrameClassification
{
uint srv_0;
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_FrameClassification> pass_FrameClassification: register( b2, space6);
const FrameClassification CreateFrameClassification()
{
	FrameClassification result;
	result.srv.frames = (pass_FrameClassification.srv_0 );
	result.uav.hi = (pass_FrameClassification.uav_0 );
	result.uav.low = (pass_FrameClassification.uav_1 );
	return result;
}
#ifndef NO_GLOBAL
static const FrameClassification frameClassification_global = CreateFrameClassification();
const FrameClassification GetFrameClassification(){ return frameClassification_global; }
#endif
