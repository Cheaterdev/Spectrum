#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_6_0:register(b0,space6);
MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	result.cb = cb_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
#endif
