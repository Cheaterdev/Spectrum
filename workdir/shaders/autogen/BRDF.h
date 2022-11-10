#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BRDF.h"
ConstantBuffer<BRDF> pass_BRDF: register( b2, space4);
const BRDF CreateBRDF()
{
	return pass_BRDF;
}
#ifndef NO_GLOBAL
static const BRDF bRDF_global = CreateBRDF();
const BRDF GetBRDF(){ return bRDF_global; }
#endif
