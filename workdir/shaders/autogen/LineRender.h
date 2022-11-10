#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
ConstantBuffer<LineRender> pass_LineRender: register( b2, space4);
const LineRender CreateLineRender()
{
	return pass_LineRender;
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
