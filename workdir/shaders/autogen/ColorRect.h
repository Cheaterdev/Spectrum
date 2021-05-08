#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ColorRect.h"
ConstantBuffer<ColorRect_cb> cb_3_0:register(b0,space3);
struct Pass_ColorRect
{
};
ConstantBuffer<Pass_ColorRect> pass_ColorRect: register( b2, space3);
const ColorRect CreateColorRect()
{
	ColorRect result;
	Pass_ColorRect pass;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const ColorRect colorRect_global = CreateColorRect();
const ColorRect GetColorRect(){ return colorRect_global; }
#endif
