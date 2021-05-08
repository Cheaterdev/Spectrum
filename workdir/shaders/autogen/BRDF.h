#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
RWTexture3D<float4> uav_3_0: register(u0, space3);
struct Pass_BRDF
{
uint uav_0;
};
ConstantBuffer<Pass_BRDF> pass_BRDF: register( b2, space3);
const BRDF CreateBRDF()
{
	BRDF result;
	Pass_BRDF pass;
	result.uav.output = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
#endif
