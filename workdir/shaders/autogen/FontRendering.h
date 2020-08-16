#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRendering.h"
Texture2D<float> srv_3_0: register(t0, space3);
Buffer<float4> srv_3_1: register(t1, space3);
FontRendering CreateFontRendering()
{
	FontRendering result;
	result.srv.tex0 = srv_3_0;
	result.srv.positions = srv_3_1;
	return result;
}
static const FontRendering fontRendering_global = CreateFontRendering();
const FontRendering GetFontRendering(){ return fontRendering_global; }
