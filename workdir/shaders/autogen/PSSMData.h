#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMData.h"
Texture2DArray<float> srv_4_0: register(t0, space4);
StructuredBuffer<Camera> srv_4_1: register(t1, space4);
PSSMData CreatePSSMData()
{
	PSSMData result;
	result.srv.light_buffer = srv_4_0;
	result.srv.light_cameras = srv_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMData pSSMData_global = CreatePSSMData();
const PSSMData GetPSSMData(){ return pSSMData_global; }
#endif
