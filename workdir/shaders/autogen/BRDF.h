#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
RWTexture3D<float4> uav_4_0: register(u0, space4);
struct Pass_BRDF
{
uint uav_0;
};
ConstantBuffer<Pass_BRDF> pass_BRDF: register( b2, space4);
const BRDF CreateBRDF()
{
	BRDF result;
	Pass_BRDF pass;
	result.uav.output = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
#endif
