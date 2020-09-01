#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Resolve.h"
ConstantBuffer<Resolve_cb> cb_2_0:register(b0,space2);
Texture2D srv_2_0: register(t0, space2);
Texture2D srv_2_1: register(t1, space2);
Resolve CreateResolve()
{
	Resolve result;
	result.cb = cb_2_0;
	result.srv.areaTex = srv_2_0;
	result.srv.searchTex = srv_2_1;
	return result;
}
#ifndef NO_GLOBAL
static const Resolve resolve_global = CreateResolve();
const Resolve GetResolve(){ return resolve_global; }
#endif
