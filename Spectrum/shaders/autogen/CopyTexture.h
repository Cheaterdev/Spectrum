#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/CopyTexture.h"
Texture2D<float4> srv_3_0: register(t0, space3);
CopyTexture CreateCopyTexture()
{
	CopyTexture result;
	result.srv.srcTex = srv_3_0;
	return result;
}
static const CopyTexture copyTexture_global = CreateCopyTexture();
const CopyTexture GetCopyTexture(){ return copyTexture_global; }
