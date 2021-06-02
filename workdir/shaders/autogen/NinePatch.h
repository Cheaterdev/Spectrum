#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D<float4> bindless[]: register(t1, space4);
#include "tables/NinePatch.h"
StructuredBuffer<vertex_input> srv_4_0: register(t0, space4);
struct Pass_NinePatch
{
uint srv_0;
};
ConstantBuffer<Pass_NinePatch> pass_NinePatch: register( b2, space4);
const NinePatch CreateNinePatch()
{
	NinePatch result;
	result.srv.vb = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const NinePatch ninePatch_global = CreateNinePatch();
const NinePatch GetNinePatch(){ return ninePatch_global; }
#endif
