#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DenoiserShadow_Fileter.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
#ifdef __spirv__
struct _CB_DenoiserShadow_Fileter { uint offset; };
static _CB_DenoiserShadow_Fileter pass_DenoiserShadow_Fileter = { _hal_push.s4 };
#else
ConstantBuffer<CB> pass_DenoiserShadow_Fileter: register(b4, space4);
#endif
 ConstantBuffer<DenoiserShadow_Fileter> CreateDenoiserShadow_Fileter()
{
	return ResourceDescriptorHeap[pass_DenoiserShadow_Fileter.offset];
}
#ifndef NO_GLOBAL
static const ConstantBuffer<DenoiserShadow_Fileter> denoiserShadow_Fileter_global = CreateDenoiserShadow_Fileter();
ConstantBuffer<DenoiserShadow_Fileter> GetDenoiserShadow_Fileter(){ return denoiserShadow_Fileter_global; }
#endif
