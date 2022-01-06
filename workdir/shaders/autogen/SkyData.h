#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyData.h"
ConstantBuffer<SkyData> pass_SkyData: register( b2, space4);
const SkyData CreateSkyData()
{
	return pass_SkyData;
}
#ifndef NO_GLOBAL
static const SkyData skyData_global = CreateSkyData();
const SkyData GetSkyData(){ return skyData_global; }
#endif
