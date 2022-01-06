#pragma once
#include "Camera.h"
struct PSSMData
{
	uint light_buffer; // Texture2DArray<float>
	uint light_cameras; // StructuredBuffer<Camera>
	Texture2DArray<float> GetLight_buffer() { return ResourceDescriptorHeap[light_buffer]; }
	StructuredBuffer<Camera> GetLight_cameras() { return ResourceDescriptorHeap[light_cameras]; }
};
