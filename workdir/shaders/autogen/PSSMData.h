#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMData.h"
Texture2DArray<float> srv_3_0: register(t0, space3);
StructuredBuffer<Camera> srv_3_1: register(t1, space3);
PSSMData CreatePSSMData()
{
	PSSMData result;
	result.srv.light_buffer = srv_3_0;
	result.srv.light_cameras = srv_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMData pSSMData_global = CreatePSSMData();
const PSSMData GetPSSMData(){ return pSSMData_global; }
#endif
