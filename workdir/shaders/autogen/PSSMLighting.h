#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMLighting.h"
ConstantBuffer<PSSMLighting> pass_PSSMLighting: register( b2, space6);
const PSSMLighting CreatePSSMLighting()
{
	return pass_PSSMLighting;
}
#ifndef NO_GLOBAL
static const PSSMLighting pSSMLighting_global = CreatePSSMLighting();
const PSSMLighting GetPSSMLighting(){ return pSSMLighting_global; }
#endif
