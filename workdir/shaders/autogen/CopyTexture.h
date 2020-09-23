#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/CopyTexture.h"
Texture2D<float4> srv_2_0: register(t0, space2);
CopyTexture CreateCopyTexture()
{
	CopyTexture result;
	result.srv.srcTex = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const CopyTexture copyTexture_global = CreateCopyTexture();
const CopyTexture GetCopyTexture(){ return copyTexture_global; }
#endif