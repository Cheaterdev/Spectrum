#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Countour.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Countour: register( b2, space4);
 ConstantBuffer<Countour> CreateCountour()
{
	return ResourceDescriptorHeap[pass_Countour.offset];
}
#ifndef NO_GLOBAL
static const Countour countour_global = CreateCountour();
const Countour GetCountour(){ return countour_global; }
#endif
