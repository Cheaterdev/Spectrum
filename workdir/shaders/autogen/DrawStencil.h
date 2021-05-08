#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
StructuredBuffer<float4> srv_3_0: register(t0, space3);
struct Pass_DrawStencil
{
uint srv_0;
};
ConstantBuffer<Pass_DrawStencil> pass_DrawStencil: register( b2, space3);
const DrawStencil CreateDrawStencil()
{
	DrawStencil result;
	Pass_DrawStencil pass;
	result.srv.vertices = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
#endif
