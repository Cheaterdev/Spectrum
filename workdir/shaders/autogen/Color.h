#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
ConstantBuffer<Color_cb> cb_4_0:register(b0,space4);
struct Pass_Color
{
};
ConstantBuffer<Pass_Color> pass_Color: register( b2, space4);
const Color CreateColor()
{
	Color result;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
#endif
