#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
StructuredBuffer<float4> srv_2_0: register(t0, space2);
DrawStencil CreateDrawStencil()
{
	DrawStencil result;
	result.srv.vertices = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
#endif
