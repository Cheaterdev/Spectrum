#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SkyData.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SkyData: register( b2, space4);
 ConstantBuffer<SkyData> CreateSkyData()
{
	return ResourceDescriptorHeap[pass_SkyData.offset];
}
#ifndef NO_GLOBAL
static const SkyData skyData_global = CreateSkyData();
const SkyData GetSkyData(){ return skyData_global; }
#endif
