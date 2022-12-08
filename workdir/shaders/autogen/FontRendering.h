#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRendering.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FontRendering: register( b2, space4);
 ConstantBuffer<FontRendering> CreateFontRendering()
{
	return ResourceDescriptorHeap[pass_FontRendering.offset];
}
#ifndef NO_GLOBAL
static const FontRendering fontRendering_global = CreateFontRendering();
const FontRendering GetFontRendering(){ return fontRendering_global; }
#endif
