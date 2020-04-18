#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D bindless[]: register(t1, space2);
#include "tables/NinePatch.h"
StructuredBuffer<vertex_input> srv_2_0: register(t0, space2);
NinePatch CreateNinePatch()
{
	NinePatch result;
	result.srv.vb = srv_2_0;
	return result;
}
static const NinePatch ninePatch_global = CreateNinePatch();
const NinePatch GetNinePatch(){ return ninePatch_global; }
