#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
ConstantBuffer<Color_cb> cb_3_0:register(b0,space3);
struct Pass_Color
{
};
ConstantBuffer<Pass_Color> pass_Color: register( b2, space3);
const Color CreateColor()
{
	Color result;
	Pass_Color pass;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
#endif
