#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FontRenderingConstants.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FontRenderingConstants: register( b2, space5);
 ConstantBuffer<FontRenderingConstants> CreateFontRenderingConstants()
{
	return ResourceDescriptorHeap[pass_FontRenderingConstants.offset];
}
#ifndef NO_GLOBAL
static const FontRenderingConstants fontRenderingConstants_global = CreateFontRenderingConstants();
const FontRenderingConstants GetFontRenderingConstants(){ return fontRenderingConstants_global; }
#endif
