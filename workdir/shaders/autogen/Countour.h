#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Countour.h"
ConstantBuffer<Countour_cb> cb_3_0:register(b0,space3);
Texture2D<float4> srv_3_0: register(t0, space3);
Countour CreateCountour()
{
	Countour result;
	result.cb = cb_3_0;
	result.srv.tex = srv_3_0;
	return result;
}
static const Countour countour_global = CreateCountour();
const Countour GetCountour(){ return countour_global; }
