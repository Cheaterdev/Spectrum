#pragma once
#include "Camera.h"
struct FrameInfo_cb
{
	Camera_cb camera;
	Camera_cb prevCamera;
};
struct FrameInfo_srv
{
	Texture2D bestFitNormals;
};
struct FrameInfo
{
	FrameInfo_cb cb;
	FrameInfo_srv srv;
	Texture2D GetBestFitNormals() { return srv.bestFitNormals; }
	Camera camera;
	Camera GetCamera() { return camera; }
	Camera prevCamera;
	Camera GetPrevCamera() { return prevCamera; }
};
 const FrameInfo CreateFrameInfo(FrameInfo_cb cb,FrameInfo_srv srv)
{
	const FrameInfo result = {cb,srv
	, CreateCamera(cb.camera)
	, CreateCamera(cb.prevCamera)
	};
	return result;
}
