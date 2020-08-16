#pragma once
#include "Camera.h"
struct FrameInfo_cb
{
	float4 time;
	Camera_cb camera;
	Camera_cb prevCamera;
};
struct FrameInfo_srv
{
	Texture2D bestFitNormals;
	Texture3D<float4> brdf;
};
struct FrameInfo
{
	FrameInfo_cb cb;
	FrameInfo_srv srv;
	float4 GetTime() { return cb.time; }
	Texture2D GetBestFitNormals() { return srv.bestFitNormals; }
	Texture3D<float4> GetBrdf() { return srv.brdf; }
	Camera GetCamera() { return CreateCamera(cb.camera); }
	Camera GetPrevCamera() { return CreateCamera(cb.prevCamera); }
};
 const FrameInfo CreateFrameInfo(FrameInfo_cb cb,FrameInfo_srv srv)
{
	const FrameInfo result = {cb,srv
	};
	return result;
}
