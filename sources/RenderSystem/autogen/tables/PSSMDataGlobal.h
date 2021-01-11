#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMDataGlobal
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float> light_buffer;
			DX12::HLSL::StructuredBuffer<Camera> light_camera;
		} &srv;
		DX12::HLSL::Texture2D<float>& GetLight_buffer() { return srv.light_buffer; }
		DX12::HLSL::StructuredBuffer<Camera>& GetLight_camera() { return srv.light_camera; }
		PSSMDataGlobal(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
