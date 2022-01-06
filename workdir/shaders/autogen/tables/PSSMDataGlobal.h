#pragma once
#include "Camera.h"
struct PSSMDataGlobal
{
	uint light_buffer; // Texture2D<float>
	uint light_camera; // StructuredBuffer<Camera>
	Texture2D<float> GetLight_buffer() { return ResourceDescriptorHeap[light_buffer]; }
	StructuredBuffer<Camera> GetLight_camera() { return ResourceDescriptorHeap[light_camera]; }
};
