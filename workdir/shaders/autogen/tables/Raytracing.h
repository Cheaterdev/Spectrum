#pragma once
struct Raytracing
{
	uint scene; // RaytracingAccelerationStructure
	RaytracingAccelerationStructure GetScene() { return ResourceDescriptorHeap[scene]; }
};
