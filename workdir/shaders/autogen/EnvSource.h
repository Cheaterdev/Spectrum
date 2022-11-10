#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
ConstantBuffer<EnvSource> pass_EnvSource: register( b2, space6);
const EnvSource CreateEnvSource()
{
	return pass_EnvSource;
}
#ifndef NO_GLOBAL
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
#endif
