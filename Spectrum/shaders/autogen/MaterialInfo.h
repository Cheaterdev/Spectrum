#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
Texture2D bindless[]: register(t0, space1);
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_1_0:register(b0,space1);
MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	result.cb = cb_1_0;
	return result;
}
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
