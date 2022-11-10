#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GBuffer.h"
ConstantBuffer<GBuffer> pass_GBuffer: register( b2, space6);
const GBuffer CreateGBuffer()
{
	return pass_GBuffer;
}
#ifndef NO_GLOBAL
static const GBuffer gBuffer_global = CreateGBuffer();
const GBuffer GetGBuffer(){ return gBuffer_global; }
#endif
