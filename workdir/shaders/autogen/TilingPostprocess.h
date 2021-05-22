#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/TilingPostprocess.h"
StructuredBuffer<uint2> srv_2_0: register(t0, space2);
struct Pass_TilingPostprocess
{
uint srv_0;
};
ConstantBuffer<Pass_TilingPostprocess> pass_TilingPostprocess: register( b2, space2);
const TilingPostprocess CreateTilingPostprocess()
{
	TilingPostprocess result;
	Pass_TilingPostprocess pass;
	result.srv.tiling.tiles = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const TilingPostprocess tilingPostprocess_global = CreateTilingPostprocess();
const TilingPostprocess GetTilingPostprocess(){ return tilingPostprocess_global; }
#endif
