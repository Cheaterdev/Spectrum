#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
StructuredBuffer<VSLine> srv_2_0: register(t0, space2);
LineRender CreateLineRender()
{
	LineRender result;
	result.srv.vb = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
