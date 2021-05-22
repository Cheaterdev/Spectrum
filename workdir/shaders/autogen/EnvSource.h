#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
TextureCube<float4> srv_6_0: register(t0, space6);
struct Pass_EnvSource
{
uint srv_0;
};
ConstantBuffer<Pass_EnvSource> pass_EnvSource: register( b2, space6);
const EnvSource CreateEnvSource()
{
	EnvSource result;
	Pass_EnvSource pass;
	result.srv.sourceTex = srv_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
#endif
