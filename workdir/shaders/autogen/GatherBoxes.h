#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherBoxes.h"
ConstantBuffer<GatherBoxes> pass_GatherBoxes: register( b2, space5);
const GatherBoxes CreateGatherBoxes()
{
	return pass_GatherBoxes;
}
#ifndef NO_GLOBAL
static const GatherBoxes gatherBoxes_global = CreateGatherBoxes();
const GatherBoxes GetGatherBoxes(){ return gatherBoxes_global; }
#endif
