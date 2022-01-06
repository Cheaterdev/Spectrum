#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawStencil.h"
ConstantBuffer<DrawStencil> pass_DrawStencil: register( b2, space4);
const DrawStencil CreateDrawStencil()
{
	return pass_DrawStencil;
}
#ifndef NO_GLOBAL
static const DrawStencil drawStencil_global = CreateDrawStencil();
const DrawStencil GetDrawStencil(){ return drawStencil_global; }
#endif
