#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
RWTexture3D<float4> uav_3_0: register(u0, space3);
BRDF CreateBRDF()
{
	BRDF result;
	result.uav.output = uav_3_0;
	return result;
}
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
