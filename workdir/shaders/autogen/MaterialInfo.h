#ifndef SLOT_8
	#define SLOT_8
#else
	#error Slot 8 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_8_0:register(b0,space8);
struct Pass_MaterialInfo
{
};
ConstantBuffer<Pass_MaterialInfo> pass_MaterialInfo: register( b2, space8);
const MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	result.cb = cb_8_0;
	return result;
}
#ifndef NO_GLOBAL
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
#endif
