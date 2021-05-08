#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Blend.h"
Texture2D<float4> srv_4_0: register(t0, space4);
struct Pass_SMAA_Blend
{
uint srv_0;
};
ConstantBuffer<Pass_SMAA_Blend> pass_SMAA_Blend: register( b2, space4);
const SMAA_Blend CreateSMAA_Blend()
{
	SMAA_Blend result;
	Pass_SMAA_Blend pass;
	result.srv.blendTex = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Blend sMAA_Blend_global = CreateSMAA_Blend();
const SMAA_Blend GetSMAA_Blend(){ return sMAA_Blend_global; }
#endif
