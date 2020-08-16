#pragma once
#include "Camera.h"
struct PSSMDataGlobal_srv
{
	Texture2D<float> light_buffer;
	StructuredBuffer<Camera> light_camera;
};
struct PSSMDataGlobal
{
	PSSMDataGlobal_srv srv;
	Texture2D<float> GetLight_buffer() { return srv.light_buffer; }
	StructuredBuffer<Camera> GetLight_camera() { return srv.light_camera; }
};
 const PSSMDataGlobal CreatePSSMDataGlobal(PSSMDataGlobal_srv srv)
{
	const PSSMDataGlobal result = {srv
	};
	return result;
}
