#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
RWStructuredBuffer<uint> uav_2_0: register(u0, space2);
PickerBuffer CreatePickerBuffer()
{
	PickerBuffer result;
	result.uav.viewBuffer = uav_2_0;
	return result;
}
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
