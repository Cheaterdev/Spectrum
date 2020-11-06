#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
StructuredBuffer<VSLine> srv_3_0: register(t0, space3);
LineRender CreateLineRender()
{
	LineRender result;
	result.srv.vb = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
