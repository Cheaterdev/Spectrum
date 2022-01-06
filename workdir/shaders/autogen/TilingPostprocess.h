#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/TilingPostprocess.h"
ConstantBuffer<TilingPostprocess> pass_TilingPostprocess: register( b2, space2);
const TilingPostprocess CreateTilingPostprocess()
{
	return pass_TilingPostprocess;
}
#ifndef NO_GLOBAL
static const TilingPostprocess tilingPostprocess_global = CreateTilingPostprocess();
const TilingPostprocess GetTilingPostprocess(){ return tilingPostprocess_global; }
#endif
