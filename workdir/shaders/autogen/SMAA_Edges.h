#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/SMAA_Edges.h"
ConstantBuffer<SMAA_Edges_cb> cb_2_0:register(b0,space2);
Texture2D srv_2_0: register(t0, space2);
Texture2D srv_2_1: register(t1, space2);
Texture2D srv_2_2: register(t2, space2);
SMAA_Edges CreateSMAA_Edges()
{
	SMAA_Edges result;
	result.cb = cb_2_0;
	result.srv.areaTex = srv_2_0;
	result.srv.searchTex = srv_2_1;
	result.srv.colorTex = srv_2_2;
	return result;
}
#ifndef NO_GLOBAL
static const SMAA_Edges sMAA_Edges_global = CreateSMAA_Edges();
const SMAA_Edges GetSMAA_Edges(){ return sMAA_Edges_global; }
#endif
