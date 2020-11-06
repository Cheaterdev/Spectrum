#ifndef SLOT_7
	#define SLOT_7
#else
	#error Slot 7 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_7_0:register(b0,space7);
MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	result.cb = cb_7_0;
	return result;
}
#ifndef NO_GLOBAL
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
#endif
