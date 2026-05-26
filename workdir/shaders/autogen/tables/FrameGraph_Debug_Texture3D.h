#pragma once
#include "sig_hlsl.hlsl"
#include "Camera.h"
struct FrameGraph_Debug_Texture3D
{
	uint3 sourceSize; // uint3
	uint source; // Texture3D<float4>
	Camera camera; // Camera
	uint3 GetSourceSize() { return sourceSize; }
	Camera GetCamera() { return camera; }
	Texture3D<float4> GetSource() { return ResourceDescriptorHeap[source]; }
};