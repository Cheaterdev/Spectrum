#pragma once
struct PickerBuffer_uav
{
	RWStructuredBuffer<uint> viewBuffer;
};
struct PickerBuffer
{
	PickerBuffer_uav uav;
	RWStructuredBuffer<uint> GetViewBuffer() { return uav.viewBuffer; }

};
 const PickerBuffer CreatePickerBuffer(PickerBuffer_uav uav)
{
	const PickerBuffer result = {uav
	};
	return result;
}
