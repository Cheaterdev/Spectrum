#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
TextureCube srv_5_0: register(t0, space5);
EnvSource CreateEnvSource()
{
	EnvSource result;
	result.srv.sourceTex = srv_5_0;
	return result;
}
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
