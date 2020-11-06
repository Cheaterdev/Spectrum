#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D bindless[]: register(t1, space3);
#include "tables/NinePatch.h"
StructuredBuffer<vertex_input> srv_3_0: register(t0, space3);
NinePatch CreateNinePatch()
{
	NinePatch result;
	result.srv.vb = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const NinePatch ninePatch_global = CreateNinePatch();
const NinePatch GetNinePatch(){ return ninePatch_global; }
#endif
