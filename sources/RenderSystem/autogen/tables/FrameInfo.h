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
			float4 sunDir;
			Camera::CB camera;
			Camera::CB prevCamera;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture2D<float4> bestFitNormals;
			Render::HLSL::Texture3D<float4> brdf;
			Render::HLSL::TextureCube<float4> sky;
		} &srv;
		float4& GetTime() { return cb.time; }
		Render::HLSL::Texture2D<float4>& GetBestFitNormals() { return srv.bestFitNormals; }
		Render::HLSL::Texture3D<float4>& GetBrdf() { return srv.brdf; }
		Render::HLSL::TextureCube<float4>& GetSky() { return srv.sky; }
		float4& GetSunDir() { return cb.sunDir; }
		Camera MapCamera() { return Camera(cb.camera); }
		Camera MapPrevCamera() { return Camera(cb.prevCamera); }
		FrameInfo(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
