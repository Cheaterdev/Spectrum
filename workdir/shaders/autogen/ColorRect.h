#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ColorRect.h"
ConstantBuffer<ColorRect_cb> cb_4_0:register(b0,space4);
struct Pass_ColorRect
{
};
ConstantBuffer<Pass_ColorRect> pass_ColorRect: register( b2, space4);
const ColorRect CreateColorRect()
{
	ColorRect result;
	Pass_ColorRect pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const ColorRect colorRect_global = CreateColorRect();
const ColorRect GetColorRect(){ return colorRect_global; }
#endif
