#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Countour.h"
ConstantBuffer<Countour> pass_Countour: register( b2, space4);
const Countour CreateCountour()
{
	return pass_Countour;
}
#ifndef NO_GLOBAL
static const Countour countour_global = CreateCountour();
const Countour GetCountour(){ return countour_global; }
#endif
