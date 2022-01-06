#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ColorRect.h"
ConstantBuffer<ColorRect> pass_ColorRect: register( b2, space4);
const ColorRect CreateColorRect()
{
	return pass_ColorRect;
}
#ifndef NO_GLOBAL
static const ColorRect colorRect_global = CreateColorRect();
const ColorRect GetColorRect(){ return colorRect_global; }
#endif
