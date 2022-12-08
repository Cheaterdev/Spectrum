#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Global.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SMAA_Global: register( b2, space4);
 ConstantBuffer<SMAA_Global> CreateSMAA_Global()
{
	return ResourceDescriptorHeap[pass_SMAA_Global.offset];
}
#ifndef NO_GLOBAL
static const SMAA_Global sMAA_Global_global = CreateSMAA_Global();
const SMAA_Global GetSMAA_Global(){ return sMAA_Global_global; }
#endif
