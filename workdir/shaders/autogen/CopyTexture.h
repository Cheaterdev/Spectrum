#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/CopyTexture.h"
Texture2D<float4> srv_4_0: register(t0, space4);
struct Pass_CopyTexture
{
uint srv_0;
};
ConstantBuffer<Pass_CopyTexture> pass_CopyTexture: register( b2, space4);
const CopyTexture CreateCopyTexture()
{
	CopyTexture result;
	result.srv.srcTex = (pass_CopyTexture.srv_0 );
	return result;
}
#ifndef NO_GLOBAL
static const CopyTexture copyTexture_global = CreateCopyTexture();
const CopyTexture GetCopyTexture(){ return copyTexture_global; }
#endif
