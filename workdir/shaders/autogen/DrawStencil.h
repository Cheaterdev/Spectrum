#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
StructuredBuffer<float4> srv_4_0: register(t0, space4);
struct Pass_DrawStencil
{
uint srv_0;
};
ConstantBuffer<Pass_DrawStencil> pass_DrawStencil: register( b2, space4);
const DrawStencil CreateDrawStencil()
{
	DrawStencil result;
	Pass_DrawStencil pass;
	result.srv.vertices = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
#endif
