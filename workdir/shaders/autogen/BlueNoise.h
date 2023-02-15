#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/BlueNoise.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_BlueNoise: register( b2, space4);
 ConstantBuffer<BlueNoise> CreateBlueNoise()
{
	return ResourceDescriptorHeap[pass_BlueNoise.offset];
}
#ifndef NO_GLOBAL
static const BlueNoise blueNoise_global = CreateBlueNoise();
const BlueNoise GetBlueNoise(){ return blueNoise_global; }
#endif
