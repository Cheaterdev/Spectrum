#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Color.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Color: register( b2, space4);
 ConstantBuffer<Color> CreateColor()
{
	return ResourceDescriptorHeap[pass_Color.offset];
}
#ifndef NO_GLOBAL
static const Color color_global = CreateColor();
const Color GetColor(){ return color_global; }
#endif
