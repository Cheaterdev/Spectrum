#ifndef SLOT_8
	#define SLOT_8
#else
	#error Slot 8 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MaterialInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_MaterialInfo: register( b2, space8);
 ConstantBuffer<MaterialInfo> CreateMaterialInfo()
{
	return ResourceDescriptorHeap[pass_MaterialInfo.offset];
}
#ifndef NO_GLOBAL
static const MaterialInfo materialInfo_global = CreateMaterialInfo();
const MaterialInfo GetMaterialInfo(){ return materialInfo_global; }
#endif
