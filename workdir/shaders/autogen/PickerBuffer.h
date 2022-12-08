#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PickerBuffer.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_PickerBuffer: register( b2, space4);
 ConstantBuffer<PickerBuffer> CreatePickerBuffer()
{
	return ResourceDescriptorHeap[pass_PickerBuffer.offset];
}
#ifndef NO_GLOBAL
static const PickerBuffer pickerBuffer_global = CreatePickerBuffer();
const PickerBuffer GetPickerBuffer(){ return pickerBuffer_global; }
#endif
