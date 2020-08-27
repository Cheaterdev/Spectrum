#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRendering.h"
Texture2D<float> srv_2_0: register(t0, space2);
Buffer<float4> srv_2_1: register(t1, space2);
FontRendering CreateFontRendering()
{
	FontRendering result;
	result.srv.tex0 = srv_2_0;
	result.srv.positions = srv_2_1;
	return result;
}
#ifndef NO_GLOBAL
static const FontRendering fontRendering_global = CreateFontRendering();
const FontRendering GetFontRendering(){ return fontRendering_global; }
#endif
