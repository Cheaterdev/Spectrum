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
			DX12::HLSL::Texture2D<float4> bestFitNormals;
			DX12::HLSL::Texture3D<float4> brdf;
			DX12::HLSL::TextureCube<float4> sky;
		} &srv;
		float4& GetTime() { return cb.time; }
		DX12::HLSL::Texture2D<float4>& GetBestFitNormals() { return srv.bestFitNormals; }
		DX12::HLSL::Texture3D<float4>& GetBrdf() { return srv.brdf; }
		DX12::HLSL::TextureCube<float4>& GetSky() { return srv.sky; }
		float4& GetSunDir() { return cb.sunDir; }
		Camera MapCamera() { return Camera(cb.camera); }
		Camera MapPrevCamera() { return Camera(cb.prevCamera); }
		FrameInfo(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
