#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_DrawStencil: register( b2, space4);
 ConstantBuffer<DrawStencil> CreateDrawStencil()
{
	return ResourceDescriptorHeap[pass_DrawStencil.offset];
}
#ifndef NO_GLOBAL
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
#endif
