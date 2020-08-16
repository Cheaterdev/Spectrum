#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
Texture2D<float> srv_4_0: register(t0, space4);
StructuredBuffer<Camera> srv_4_1: register(t1, space4);
PSSMDataGlobal CreatePSSMDataGlobal()
{
	PSSMDataGlobal result;
	result.srv.light_buffer = srv_4_0;
	result.srv.light_camera = srv_4_1;
	return result;
}
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
