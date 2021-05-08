#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
StructuredBuffer<VSLine> srv_3_0: register(t0, space3);
struct Pass_LineRender
{
uint srv_0;
};
ConstantBuffer<Pass_LineRender> pass_LineRender: register( b2, space3);
const LineRender CreateLineRender()
{
	LineRender result;
	Pass_LineRender pass;
	result.srv.vb = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
