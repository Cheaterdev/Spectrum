#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
ConstantBuffer<Color_cb> cb_2_0:register(b0,space2);
Color CreateColor()
{
	Color result;
	result.cb = cb_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
#endif
