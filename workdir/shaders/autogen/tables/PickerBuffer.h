#pragma once
struct PickerBuffer
{
	uint viewBuffer; // RWStructuredBuffer<uint>
	RWStructuredBuffer<uint> GetViewBuffer() { return ResourceDescriptorHeap[viewBuffer]; }
};
