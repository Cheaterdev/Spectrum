#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/DownsampleDepth.h"
ConstantBuffer<DownsampleDepth> pass_DownsampleDepth: register( b2, space4);
const DownsampleDepth CreateDownsampleDepth()
{
	return pass_DownsampleDepth;
}
#ifndef NO_GLOBAL
static const DownsampleDepth downsampleDepth_global = CreateDownsampleDepth();
const DownsampleDepth GetDownsampleDepth(){ return downsampleDepth_global; }
#endif
