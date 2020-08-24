#pragma once
#include "Camera.h"
struct PSSMData_srv
{
	Texture2DArray<float> light_buffer;
	StructuredBuffer<Camera> light_cameras;
};
struct PSSMData
{
	PSSMData_srv srv;
	Texture2DArray<float> GetLight_buffer() { return srv.light_buffer; }
	StructuredBuffer<Camera> GetLight_cameras() { return srv.light_cameras; }

};
 const PSSMData CreatePSSMData(PSSMData_srv srv)
{
	const PSSMData result = {srv
	};
	return result;
}
