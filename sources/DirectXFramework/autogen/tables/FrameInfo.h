#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameInfo
	{
		float4 time;
		float4 sunDir;
		Render::HLSL::Texture2D<float4> bestFitNormals;
		Render::HLSL::Texture3D<float4> brdf;
		Render::HLSL::TextureCube<float4> sky;
		Camera camera;
		Camera prevCamera;
		float4& GetTime() { return time; }
		Render::HLSL::Texture2D<float4>& GetBestFitNormals() { return bestFitNormals; }
		Render::HLSL::Texture3D<float4>& GetBrdf() { return brdf; }
		Render::HLSL::TextureCube<float4>& GetSky() { return sky; }
		float4& GetSunDir() { return sunDir; }
		Camera& MapCamera() { return camera; }
		Camera& MapPrevCamera() { return prevCamera; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(time);
			compiler.compile(sunDir);
			compiler.compile(bestFitNormals);
			compiler.compile(brdf);
			compiler.compile(sky);
			compiler.compile(camera);
			compiler.compile(prevCamera);
		}
	};
	#pragma pack(pop)
}
