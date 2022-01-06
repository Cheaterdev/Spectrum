#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
ConstantBuffer<PickerBuffer> pass_PickerBuffer: register( b2, space4);
const PickerBuffer CreatePickerBuffer()
{
	return pass_PickerBuffer;
}
#ifndef NO_GLOBAL
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
#endif
