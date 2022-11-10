#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
ConstantBuffer<Color> pass_Color: register( b2, space4);
const Color CreateColor()
{
	return pass_Color;
}
#ifndef NO_GLOBAL
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
#endif
