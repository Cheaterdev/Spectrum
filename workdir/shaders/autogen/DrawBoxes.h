#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawBoxes.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DrawBoxes: register( b2, space5);
 ConstantBuffer<DrawBoxes> CreateDrawBoxes()
{
	return ResourceDescriptorHeap[pass_DrawBoxes.offset];
}
#ifndef NO_GLOBAL
static const DrawBoxes drawBoxes_global = CreateDrawBoxes();
const DrawBoxes GetDrawBoxes(){ return drawBoxes_global; }
#endif
