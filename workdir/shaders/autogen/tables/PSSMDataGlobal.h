#pragma once
#include "Camera.h"
struct PSSMDataGlobal_srv
{
	uint light_buffer; // Texture2D<float>
	uint light_camera; // StructuredBuffer<Camera>
};
struct PSSMDataGlobal
{
	PSSMDataGlobal_srv srv;
	Texture2D<float> GetLight_buffer() { return ResourceDescriptorHeap[srv.light_buffer]; }
	StructuredBuffer<Camera> GetLight_camera() { return ResourceDescriptorHeap[srv.light_camera]; }

};
 const PSSMDataGlobal CreatePSSMDataGlobal(PSSMDataGlobal_srv srv)
{
	const PSSMDataGlobal result = {srv
	};
	return result;
}
