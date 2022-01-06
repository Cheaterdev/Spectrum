#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingGlyphs.h"
ConstantBuffer<FontRenderingGlyphs> pass_FontRenderingGlyphs: register( b2, space6);
const FontRenderingGlyphs CreateFontRenderingGlyphs()
{
	return pass_FontRenderingGlyphs;
}
#ifndef NO_GLOBAL
static const FontRenderingGlyphs fontRenderingGlyphs_global = CreateFontRenderingGlyphs();
const FontRenderingGlyphs GetFontRenderingGlyphs(){ return fontRenderingGlyphs_global; }
#endif
