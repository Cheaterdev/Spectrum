#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Global.h"
ConstantBuffer<SMAA_Global> pass_SMAA_Global: register( b2, space4);
const SMAA_Global CreateSMAA_Global()
{
	return pass_SMAA_Global;
}
#ifndef NO_GLOBAL
static const SMAA_Global sMAA_Global_global = CreateSMAA_Global();
const SMAA_Global GetSMAA_Global(){ return sMAA_Global_global; }
#endif
