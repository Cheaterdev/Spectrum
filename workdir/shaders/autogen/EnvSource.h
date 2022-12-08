#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvSource.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_EnvSource: register( b2, space6);
 ConstantBuffer<EnvSource> CreateEnvSource()
{
	return ResourceDescriptorHeap[pass_EnvSource.offset];
}
#ifndef NO_GLOBAL
static const EnvSource envSource_global = CreateEnvSource();
const EnvSource GetEnvSource(){ return envSource_global; }
#endif
