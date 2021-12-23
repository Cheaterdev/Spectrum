#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/LineRender.h"
StructuredBuffer<VSLine> srv_4_0: register(t0, space4);
struct Pass_LineRender
{
uint srv_0;
};
ConstantBuffer<Pass_LineRender> pass_LineRender: register( b2, space4);
const LineRender CreateLineRender()
{
	LineRender result;
	result.srv.vb = (pass_LineRender.srv_0 );
	return result;
}
#ifndef NO_GLOBAL
static const LineRender lineRender_global = CreateLineRender();
const LineRender GetLineRender(){ return lineRender_global; }
#endif
