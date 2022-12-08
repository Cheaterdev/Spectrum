#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MipMapping.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_MipMapping: register( b2, space4);
 ConstantBuffer<MipMapping> CreateMipMapping()
{
	return ResourceDescriptorHeap[pass_MipMapping.offset];
}
#ifndef NO_GLOBAL
static const MipMapping mipMapping_global = CreateMipMapping();
const MipMapping GetMipMapping(){ return mipMapping_global; }
#endif
