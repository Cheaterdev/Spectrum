#pragma once
struct Raytracing
{
	uint scene; // RaytracingAccelerationStructure
	uint index_buffer; // StructuredBuffer<uint>
	RaytracingAccelerationStructure GetScene() { return ResourceDescriptorHeap[scene]; }
	StructuredBuffer<uint> GetIndex_buffer() { return ResourceDescriptorHeap[index_buffer]; }
};
