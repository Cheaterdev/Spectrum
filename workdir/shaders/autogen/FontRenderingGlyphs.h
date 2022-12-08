#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingGlyphs.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FontRenderingGlyphs: register( b2, space6);
 ConstantBuffer<FontRenderingGlyphs> CreateFontRenderingGlyphs()
{
	return ResourceDescriptorHeap[pass_FontRenderingGlyphs.offset];
}
#ifndef NO_GLOBAL
static const FontRenderingGlyphs fontRenderingGlyphs_global = CreateFontRenderingGlyphs();
const FontRenderingGlyphs GetFontRenderingGlyphs(){ return fontRenderingGlyphs_global; }
#endif
