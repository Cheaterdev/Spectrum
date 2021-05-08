#ifndef SLOT_7
	#define SLOT_7
#else
	#error Slot 7 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MaterialInfo.h"
ConstantBuffer<MaterialInfo_cb> cb_7_0:register(b0,space7);
struct Pass_MaterialInfo
{
};
ConstantBuffer<Pass_MaterialInfo> pass_MaterialInfo: register( b2, space7);
const MaterialInfo CreateMaterialInfo()
{
	MaterialInfo result;
	Pass_MaterialInfo pass;
	result.cb = cb_7_0;
	return result;
}
#ifndef NO_GLOBAL
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
#endif
