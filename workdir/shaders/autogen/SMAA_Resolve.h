#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Resolve.h"
Texture2D srv_2_0: register(t0, space2);
SMAA_Resolve CreateSMAA_Resolve()
{
	SMAA_Resolve result;
	result.srv.blendTex = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Resolve sMAA_Resolve_global = CreateSMAA_Resolve();
const SMAA_Resolve GetSMAA_Resolve(){ return sMAA_Resolve_global; }
#endif
