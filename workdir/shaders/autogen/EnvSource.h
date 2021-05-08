#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
TextureCube<float4> srv_5_0: register(t0, space5);
struct Pass_EnvSource
{
uint srv_0;
};
ConstantBuffer<Pass_EnvSource> pass_EnvSource: register( b2, space5);
const EnvSource CreateEnvSource()
{
	EnvSource result;
	Pass_EnvSource pass;
	result.srv.sourceTex = srv_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
#endif
