#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/CopyTexture.h"
ConstantBuffer<CopyTexture> pass_CopyTexture: register( b2, space4);
const CopyTexture CreateCopyTexture()
{
	return pass_CopyTexture;
}
#ifndef NO_GLOBAL
static const CopyTexture copyTexture_global = CreateCopyTexture();
const CopyTexture GetCopyTexture(){ return copyTexture_global; }
#endif
