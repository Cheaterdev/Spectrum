#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingGlyphs.h"
StructuredBuffer<Glyph> srv_6_0: register(t0, space6);
struct Pass_FontRenderingGlyphs
{
uint srv_0;
};
ConstantBuffer<Pass_FontRenderingGlyphs> pass_FontRenderingGlyphs: register( b2, space6);
const FontRenderingGlyphs CreateFontRenderingGlyphs()
{
	FontRenderingGlyphs result;
	result.srv.data = srv_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const FontRenderingGlyphs fontRenderingGlyphs_global = CreateFontRenderingGlyphs();
const FontRenderingGlyphs GetFontRenderingGlyphs(){ return fontRenderingGlyphs_global; }
#endif
