#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingConstants.h"
ConstantBuffer<FontRenderingConstants> pass_FontRenderingConstants: register( b2, space5);
const FontRenderingConstants CreateFontRenderingConstants()
{
	return pass_FontRenderingConstants;
}
#ifndef NO_GLOBAL
static const FontRenderingConstants fontRenderingConstants_global = CreateFontRenderingConstants();
const FontRenderingConstants GetFontRenderingConstants(){ return fontRenderingConstants_global; }
#endif
