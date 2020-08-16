#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameInfo
	{
		struct CB
		{
			float4 time;
			Camera::CB camera;
			Camera::CB prevCamera;
		} &cb;
		struct SRV
		{
			Render::Handle bestFitNormals;
			Render::Handle brdf;
			Camera::SRV camera;
			Camera::SRV prevCamera;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetTime() { return cb.time; }
		Render::Handle& GetBestFitNormals() { return srv.bestFitNormals; }
		Render::Handle& GetBrdf() { return srv.brdf; }
		Camera MapCamera() { return Camera(cb.camera); }
		Camera MapPrevCamera() { return Camera(cb.prevCamera); }
		FrameInfo(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
