#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Blend.h"
Texture2D srv_3_0: register(t0, space3);
SMAA_Blend CreateSMAA_Blend()
{
	SMAA_Blend result;
	result.srv.blendTex = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Blend sMAA_Blend_global = CreateSMAA_Blend();
const SMAA_Blend GetSMAA_Blend(){ return sMAA_Blend_global; }
#endif
