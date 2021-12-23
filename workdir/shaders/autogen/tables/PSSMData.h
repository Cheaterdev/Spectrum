#pragma once
#include "Camera.h"
struct PSSMData_srv
{
	uint light_buffer; // Texture2DArray<float>
	uint light_cameras; // StructuredBuffer<Camera>
};
struct PSSMData
{
	PSSMData_srv srv;
	Texture2DArray<float> GetLight_buffer() { return ResourceDescriptorHeap[srv.light_buffer]; }
	StructuredBuffer<Camera> GetLight_cameras() { return ResourceDescriptorHeap[srv.light_cameras]; }

};
 const PSSMData CreatePSSMData(PSSMData_srv srv)
{
	const PSSMData result = {srv
	};
	return result;
}
