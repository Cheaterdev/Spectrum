#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/ColorRect.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_ColorRect: register( b2, space4);
 ConstantBuffer<ColorRect> CreateColorRect()
{
	return ResourceDescriptorHeap[pass_ColorRect.offset];
}
#ifndef NO_GLOBAL
static const ColorRect colorRect_global = CreateColorRect();
const ColorRect GetColorRect(){ return colorRect_global; }
#endif
