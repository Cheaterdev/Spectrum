#pragma once
#include "sig_hlsl.hlsl"
struct Raytracing
{
	uint scene; // RaytracingAccelerationStructure
	RaytracingAccelerationStructure GetScene() { return ResourceDescriptorHeap[scene]; }
};
