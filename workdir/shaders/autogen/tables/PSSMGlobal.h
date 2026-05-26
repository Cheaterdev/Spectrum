#pragma once
#include "sig_hlsl.hlsl"
#include "Camera.h"
struct PSSMGlobal
{
	uint global_depth; // Texture
	uint global_camera; // StructuredBuffer<Camera>
	Texture GetGlobal_depth() { return ResourceDescriptorHeap[global_depth]; }
	StructuredBuffer<Camera> GetGlobal_camera() { return ResourceDescriptorHeap[global_camera]; }
};