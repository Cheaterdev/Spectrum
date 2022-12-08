#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMLighting.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_PSSMLighting: register( b2, space6);
 ConstantBuffer<PSSMLighting> CreatePSSMLighting()
{
	return ResourceDescriptorHeap[pass_PSSMLighting.offset];
}
#ifndef NO_GLOBAL
static const PSSMLighting pSSMLighting_global = CreatePSSMLighting();
const PSSMLighting GetPSSMLighting(){ return pSSMLighting_global; }
#endif
