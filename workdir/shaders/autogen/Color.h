#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
ConstantBuffer<Color_cb> cb_3_0:register(b0,space3);
Color CreateColor()
{
	Color result;
	result.cb = cb_3_0;
	return result;
}
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
