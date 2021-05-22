#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
RWStructuredBuffer<uint> uav_4_0: register(u0, space4);
struct Pass_PickerBuffer
{
uint uav_0;
};
ConstantBuffer<Pass_PickerBuffer> pass_PickerBuffer: register( b2, space4);
const PickerBuffer CreatePickerBuffer()
{
	PickerBuffer result;
	Pass_PickerBuffer pass;
	result.uav.viewBuffer = uav_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
#endif
