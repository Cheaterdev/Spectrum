#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMData.h"
Texture2DArray<float> srv_5_0: register(t0, space5);
StructuredBuffer<Camera> srv_5_1: register(t1, space5);
struct Pass_PSSMData
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_PSSMData> pass_PSSMData: register( b2, space5);
const PSSMData CreatePSSMData()
{
	PSSMData result;
	result.srv.light_buffer = (pass_PSSMData.srv_0 );
	result.srv.light_cameras = (pass_PSSMData.srv_1 );
	return result;
}
#ifndef NO_GLOBAL
static const PSSMData pSSMData_global = CreatePSSMData();
const PSSMData GetPSSMData(){ return pSSMData_global; }
#endif
