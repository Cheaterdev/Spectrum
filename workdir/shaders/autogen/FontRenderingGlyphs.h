#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingGlyphs.h"
StructuredBuffer<Glyph> srv_5_0: register(t0, space5);
FontRenderingGlyphs CreateFontRenderingGlyphs()
{
	FontRenderingGlyphs result;
	result.srv.data = srv_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const FontRenderingGlyphs fontRenderingGlyphs_global = CreateFontRenderingGlyphs();
const FontRenderingGlyphs GetFontRenderingGlyphs(){ return fontRenderingGlyphs_global; }
#endif
