#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
RWTexture3D<float4> uav_2_0: register(u0, space2);
BRDF CreateBRDF()
{
	BRDF result;
	result.uav.output = uav_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
#endif
