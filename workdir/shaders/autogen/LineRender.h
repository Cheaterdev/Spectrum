#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_LineRender: register( b2, space4);
 ConstantBuffer<LineRender> CreateLineRender()
{
	return ResourceDescriptorHeap[pass_LineRender.offset];
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
