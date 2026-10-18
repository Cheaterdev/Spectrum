#include "../autogen/TestAppendData.h"

// One thread per item: a plain write (proves the buffer UAV path) and an Append (the counted-buffer path
// VSM's tile lists use). Test.HAL.Compute checks both from the CPU.
[numthreads(64, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	if (id.x >= GetTestAppendData().GetCount())
		return;

	GetTestAppendData().GetPlain()[id.x] = id.x + 1;
	InterlockedAdd(GetTestAppendData().GetAtomic_counter()[0], 1);
	GetTestAppendData().GetItems().Append(uint2(id.x, id.x * 2 + 1));
}
