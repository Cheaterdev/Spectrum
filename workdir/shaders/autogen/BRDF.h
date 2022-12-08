#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_BRDF: register( b2, space4);
 ConstantBuffer<BRDF> CreateBRDF()
{
	return ResourceDescriptorHeap[pass_BRDF.offset];
}
#ifndef NO_GLOBAL
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
#endif
