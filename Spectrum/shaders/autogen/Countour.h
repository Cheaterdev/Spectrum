#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Countour.h"
ConstantBuffer<Countour_cb> cb_2_0:register(b0,space2);
Texture2D<float4> srv_2_0: register(t0, space2);
Countour CreateCountour()
{
	Countour result;
	result.cb = cb_2_0;
	result.srv.tex = srv_2_0;
	return result;
}
static const Countour countour_global = CreateCountour();
const Countour GetCountour(){ return countour_global; }
