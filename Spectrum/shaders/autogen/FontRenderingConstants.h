#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingConstants.h"
ConstantBuffer<FontRenderingConstants_cb> cb_3_0:register(b0,space3);
FontRenderingConstants CreateFontRenderingConstants()
{
	FontRenderingConstants result;
	result.cb = cb_3_0;
	return result;
}
static const FontRenderingConstants fontRenderingConstants_global = CreateFontRenderingConstants();
const FontRenderingConstants GetFontRenderingConstants(){ return fontRenderingConstants_global; }
