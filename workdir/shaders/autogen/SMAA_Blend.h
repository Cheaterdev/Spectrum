#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Blend.h"
Texture2D<float4> srv_5_0: register(t0, space5);
struct Pass_SMAA_Blend
{
uint srv_0;
};
ConstantBuffer<Pass_SMAA_Blend> pass_SMAA_Blend: register( b2, space5);
const SMAA_Blend CreateSMAA_Blend()
{
	SMAA_Blend result;
	result.srv.blendTex = (pass_SMAA_Blend.srv_0 );
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Blend sMAA_Blend_global = CreateSMAA_Blend();
const SMAA_Blend GetSMAA_Blend(){ return sMAA_Blend_global; }
#endif
