#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_2_0:register(b0,space2);
MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	result.cb = cb_2_0;
	return result;
}
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
