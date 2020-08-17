#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
StructuredBuffer<float4> srv_3_0: register(t0, space3);
DrawStencil CreateDrawStencil()
{
	DrawStencil result;
	result.srv.vertices = srv_3_0;
	return result;
}
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
