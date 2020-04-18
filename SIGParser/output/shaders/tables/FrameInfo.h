#pragma once
#include "Camera.h"
struct FrameInfo_cb
{
	Camera_cb camera;
	Camera_cb prevCamera;
};
struct FrameInfo
{
	FrameInfo_cb cb;
	Camera camera;
	Camera GetCamera() { return camera; }
	Camera prevCamera;
	Camera GetPrevCamera() { return prevCamera; }
};
 const FrameInfo CreateFrameInfo(FrameInfo_cb cb)
{
	const FrameInfo result = {cb
	, CreateCamera(cb.camera)
	, CreateCamera(cb.prevCamera)
	};
	return result;
}
