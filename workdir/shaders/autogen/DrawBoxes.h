#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DrawBoxes.h"
ConstantBuffer<DrawBoxes> pass_DrawBoxes: register( b2, space5);
const DrawBoxes CreateDrawBoxes()
{
	return pass_DrawBoxes;
}
#ifndef NO_GLOBAL
static const DrawBoxes drawBoxes_global = CreateDrawBoxes();
const DrawBoxes GetDrawBoxes(){ return drawBoxes_global; }
#endif
