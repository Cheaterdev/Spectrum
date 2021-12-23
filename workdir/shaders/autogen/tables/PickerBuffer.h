#pragma once
struct PickerBuffer_uav
{
	uint viewBuffer; // RWStructuredBuffer<uint>
};
struct PickerBuffer
{
	PickerBuffer_uav uav;
	RWStructuredBuffer<uint> GetViewBuffer() { return ResourceDescriptorHeap[uav.viewBuffer]; }

};
 const PickerBuffer CreatePickerBuffer(PickerBuffer_uav uav)
{
	const PickerBuffer result = {uav
	};
	return result;
}
