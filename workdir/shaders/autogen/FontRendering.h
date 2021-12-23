#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRendering.h"
Texture2D<float> srv_4_0: register(t0, space4);
Buffer<float4> srv_4_1: register(t1, space4);
struct Pass_FontRendering
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_FontRendering> pass_FontRendering: register( b2, space4);
const FontRendering CreateFontRendering()
{
	FontRendering result;
	result.srv.tex0 = (pass_FontRendering.srv_0 );
	result.srv.positions = (pass_FontRendering.srv_1 );
	return result;
}
#ifndef NO_GLOBAL
static const FontRendering fontRendering_global = CreateFontRendering();
const FontRendering GetFontRendering(){ return fontRendering_global; }
#endif
