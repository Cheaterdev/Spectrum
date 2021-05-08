#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassification.h"
Texture2D<float> srv_5_0: register(t0, space5);
AppendStructuredBuffer<uint2> uav_5_0: register(u0, space5);
AppendStructuredBuffer<uint2> uav_5_1: register(u1, space5);
struct Pass_FrameClassification
{
uint srv_0;
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_FrameClassification> pass_FrameClassification: register( b2, space5);
const FrameClassification CreateFrameClassification()
{
	FrameClassification result;
	Pass_FrameClassification pass;
	result.srv.frames = srv_5_0;
	result.uav.hi = uav_5_0;
	result.uav.low = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const FrameClassification frameClassification_global = CreateFrameClassification();
const FrameClassification GetFrameClassification(){ return frameClassification_global; }
#endif
