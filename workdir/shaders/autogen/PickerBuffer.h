#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
RWStructuredBuffer<uint> uav_3_0: register(u0, space3);
struct Pass_PickerBuffer
{
uint uav_0;
};
ConstantBuffer<Pass_PickerBuffer> pass_PickerBuffer: register( b2, space3);
const PickerBuffer CreatePickerBuffer()
{
	PickerBuffer result;
	Pass_PickerBuffer pass;
	result.uav.viewBuffer = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
#endif
