#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRendering.h"
ConstantBuffer<FontRendering> pass_FontRendering: register( b2, space4);
const FontRendering CreateFontRendering()
{
	return pass_FontRendering;
}
#ifndef NO_GLOBAL
static const FontRendering fontRendering_global = CreateFontRendering();
const FontRendering GetFontRendering(){ return fontRendering_global; }
#endif
