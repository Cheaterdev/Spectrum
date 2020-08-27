#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
Texture2D<float> srv_3_0: register(t0, space3);
StructuredBuffer<Camera> srv_3_1: register(t1, space3);
PSSMDataGlobal CreatePSSMDataGlobal()
{
	PSSMDataGlobal result;
	result.srv.light_buffer = srv_3_0;
	result.srv.light_camera = srv_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
#endif
