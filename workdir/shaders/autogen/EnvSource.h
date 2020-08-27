#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
TextureCube srv_4_0: register(t0, space4);
EnvSource CreateEnvSource()
{
	EnvSource result;
	result.srv.sourceTex = srv_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
#endif
