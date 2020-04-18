#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/Material.h"
ConstantBuffer<Material_cb> cb_1_0:register(b0,space1);
Material CreateMaterial()
{
	Material result;
	result.cb = cb_1_0;
	return result;
}
static const Material material_global = CreateMaterial();
const Material GetMaterial(){ return material_global; }
