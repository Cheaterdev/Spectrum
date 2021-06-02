#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Countour.h"
ConstantBuffer<Countour_cb> cb_4_0:register(b0,space4);
Texture2D<float4> srv_4_0: register(t0, space4);
struct Pass_Countour
{
uint srv_0;
};
ConstantBuffer<Pass_Countour> pass_Countour: register( b2, space4);
const Countour CreateCountour()
{
	Countour result;
	result.cb = cb_4_0;
	result.srv.tex = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const Countour countour_global = CreateCountour();
const Countour GetCountour(){ return countour_global; }
#endif
