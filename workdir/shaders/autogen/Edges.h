#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Edges.h"
Texture2D srv_2_0: register(t0, space2);
Texture2D srv_2_1: register(t1, space2);
Texture2D srv_2_2: register(t2, space2);
Edges CreateEdges()
{
	Edges result;
	result.srv.areaTex = srv_2_0;
	result.srv.searchTex = srv_2_1;
	result.srv.colorTex = srv_2_2;
	return result;
}
#ifndef NO_GLOBAL
static const Edges edges_global = CreateEdges();
const Edges GetEdges(){ return edges_global; }
#endif
