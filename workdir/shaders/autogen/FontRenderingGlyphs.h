#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingGlyphs.h"
StructuredBuffer<Glyph> srv_4_0: register(t0, space4);
FontRenderingGlyphs CreateFontRenderingGlyphs()
{
	FontRenderingGlyphs result;
	result.srv.data = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const FontRenderingGlyphs fontRenderingGlyphs_global = CreateFontRenderingGlyphs();
const FontRenderingGlyphs GetFontRenderingGlyphs(){ return fontRenderingGlyphs_global; }
#endif
