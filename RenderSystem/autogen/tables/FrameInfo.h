#pragma once
#include "Camera.h"
namespace Table 
{
	struct FrameInfo
	{
		struct CB
		{
			Camera::CB camera;
			Camera::CB prevCamera;
		} &cb;
		struct SRV
		{
			Render::Handle bestFitNormals;
			Camera::SRV camera;
			Camera::SRV prevCamera;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetBestFitNormals() { return srv.bestFitNormals; }
		Camera MapCamera() { return Camera(cb.camera); }
		Camera MapPrevCamera() { return Camera(cb.prevCamera); }
		FrameInfo(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
}
