#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
RWStructuredBuffer<uint> uav_3_0: register(u0, space3);
PickerBuffer CreatePickerBuffer()
{
	PickerBuffer result;
	result.uav.viewBuffer = uav_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
#endif
