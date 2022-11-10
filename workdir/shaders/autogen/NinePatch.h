#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/NinePatch.h"
ConstantBuffer<NinePatch> pass_NinePatch: register( b2, space4);
const NinePatch CreateNinePatch()
{
	return pass_NinePatch;
}
#ifndef NO_GLOBAL
static const NinePatch ninePatch_global = CreateNinePatch();
const NinePatch GetNinePatch(){ return ninePatch_global; }
#endif
