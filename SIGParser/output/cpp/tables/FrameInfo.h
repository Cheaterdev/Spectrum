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
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		Camera MapCamera() { return Camera(cb.camera); }
		Camera MapPrevCamera() { return Camera(cb.prevCamera); }
		FrameInfo(CB&cb) :cb(cb){}
	};
}
