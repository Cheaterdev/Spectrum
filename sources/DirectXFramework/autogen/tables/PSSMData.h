#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMData
	{
		struct SRV
		{
			Render::HLSL::Texture2DArray<float> light_buffer;
			Render::HLSL::StructuredBuffer<Camera> light_cameras;
		} &srv;
		Render::HLSL::Texture2DArray<float>& GetLight_buffer() { return srv.light_buffer; }
		Render::HLSL::StructuredBuffer<Camera>& GetLight_cameras() { return srv.light_cameras; }
		PSSMData(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
