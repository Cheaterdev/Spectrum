#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/CopyTexture.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_CopyTexture: register( b2, space4);
 ConstantBuffer<CopyTexture> CreateCopyTexture()
{
	return ResourceDescriptorHeap[pass_CopyTexture.offset];
}
#ifndef NO_GLOBAL
static const CopyTexture copyTexture_global = CreateCopyTexture();
const CopyTexture GetCopyTexture(){ return copyTexture_global; }
#endif
