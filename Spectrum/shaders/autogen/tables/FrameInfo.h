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
	Camera GetCamera() { return CreateCamera(cb.camera); }
	Camera GetPrevCamera() { return CreateCamera(cb.prevCamera); }
};
 const FrameInfo CreateFrameInfo(FrameInfo_cb cb,FrameInfo_srv srv)
{
	const FrameInfo result = {cb,srv
	};
	return result;
}
