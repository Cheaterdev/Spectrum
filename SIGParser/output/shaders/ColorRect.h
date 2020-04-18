#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ColorRect.h"
ConstantBuffer<ColorRect_cb> cb_2_0:register(b0,space2);
ColorRect CreateColorRect()
{
	ColorRect result;
	result.cb = cb_2_0;
	return result;
}
static const ColorRect colorRect_global = CreateColorRect();
const ColorRect GetColorRect(){ return colorRect_global; }
