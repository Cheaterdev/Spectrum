#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingConstants.h"
ConstantBuffer<FontRenderingConstants_cb> cb_4_0:register(b0,space4);
FontRenderingConstants CreateFontRenderingConstants()
{
	FontRenderingConstants result;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const FontRenderingConstants fontRenderingConstants_global = CreateFontRenderingConstants();
const FontRenderingConstants GetFontRenderingConstants(){ return fontRenderingConstants_global; }
#endif
