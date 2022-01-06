#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MipMapping.h"
ConstantBuffer<MipMapping> pass_MipMapping: register( b2, space4);
const MipMapping CreateMipMapping()
{
	return pass_MipMapping;
}
#ifndef NO_GLOBAL
static const MipMapping mipMapping_global = CreateMipMapping();
const MipMapping GetMipMapping(){ return mipMapping_global; }
#endif
